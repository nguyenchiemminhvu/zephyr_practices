#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <zephyr/timing/timing.h>

#include <string.h>

LOG_MODULE_REGISTER(main);

static volatile uint32_t bytes_received = 0;
static volatile uint32_t sentence_parsed = 0;

static const struct device *gnss_uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_gnss_uart));

RING_BUF_DECLARE(gnss_ringbuf, 512);

K_SEM_DEFINE(gnss_sem, 0, 1);

static void gnss_uart_irq_callback(const struct device *dev, void *user_data)
{
    timing_t start_time, end_time;
    start_time = timing_counter_get();
    
    uart_irq_update(dev);
    if (!uart_irq_rx_ready(dev))
    {
        return;
    }

    uint8_t local_buf[128];
    memset(local_buf, 0, sizeof(local_buf));
    int bytes_read = uart_fifo_read(dev, local_buf, sizeof(local_buf));
    if (bytes_read > 0)
    {
        bytes_received += bytes_read;
        if (ring_buf_put(&gnss_ringbuf, local_buf, bytes_read) < bytes_read)
        {
            LOG_WRN("Ring buffer overflow, some data lost");
        }

        k_sem_give(&gnss_sem);
    }

    end_time = timing_counter_get();
    uint32_t cycles = timing_cycles_get(&start_time, &end_time);
    uint32_t elapsed_time = timing_cycles_to_ns(cycles);
    timing_stop();
    printk("Read %d bytes in %u us\n", bytes_read, elapsed_time / 1000);
}

static char line_buf[256];
static size_t line_pos = 0;
void gnss_parser_thread(void *arg1, void *arg2, void *arg3)
{
    uint8_t byte;
    while (1)
    {
        k_sem_take(&gnss_sem, K_FOREVER);

        // drain ring buffer
        while (ring_buf_get(&gnss_ringbuf, &byte, 1) > 0)
        {
            if (byte == '$')
            {
                /* Start of new NMEA sentence */
                line_pos = 0;
                line_buf[line_pos++] = byte;
            }
            else if ((byte == '\n') && (line_pos > 0))
            {
                line_buf[line_pos] = '\0';

                // Pass to NMEA parser library
                LOG_INF("NMEA: %s", line_buf);
                // nmea_parser.feed(line_buf, line_pos);
                
                sentence_parsed++;
                line_pos = 0;
            }
            else if (line_pos > 0 && line_pos < sizeof(line_buf) - 1)
            {
                line_buf[line_pos++] = byte;
            }
        }
    }
}

K_THREAD_DEFINE(gnss_parser_thread_id, 2048, gnss_parser_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    LOG_INF("=== UART GPS Demo (Interrupt-Driven) ===");
    LOG_INF("Expecting Ublox M8 on UART1 (GPIO4/5) at 9600 baud");

    timing_init();  // Initialize the timing subsystem
    timing_start(); // Start timing measurement

    if (!device_is_ready(gnss_uart_dev))
    {
        LOG_ERR("GNSS UART device not ready");
        return -1;
    }

    uart_irq_callback_user_data_set(gnss_uart_dev, gnss_uart_irq_callback, NULL);
    uart_irq_rx_enable(gnss_uart_dev);

    while (1)
    {
        k_sleep(K_SECONDS(5));
        LOG_INF("Bytes received: %u, Sentences parsed: %u", bytes_received, sentence_parsed);
    }
    return 0;
}