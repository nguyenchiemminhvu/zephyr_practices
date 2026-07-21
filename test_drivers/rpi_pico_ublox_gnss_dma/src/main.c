#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <zephyr/timing/timing.h>

#include <string.h>

LOG_MODULE_REGISTER(main);

static const struct device *gnss_uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_gnss_uart));
static volatile uint32_t bytes_received = 0;
static volatile uint32_t sentence_parsed = 0;

RING_BUF_DECLARE(gnss_ringbuf, 512);
K_SEM_DEFINE(gnss_sem, 0, 1);



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

    // setup UART DMA RX (async API)


    while (1)
    {
        k_sleep(K_SECONDS(5));
        LOG_INF("Bytes received: %u, Sentences parsed: %u", bytes_received, sentence_parsed);
    }
    return 0;
}