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

#define DMA_BUFFER_SIZE 128
static uint8_t uart_rx_buffer_a[DMA_BUFFER_SIZE];
static uint8_t uart_rx_buffer_b[DMA_BUFFER_SIZE];

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

static void uart_async_callback(const struct device *dev, struct uart_event *evt, void *user_data)
{
    int ret = 0;
    uint32_t written = 0;
    switch (evt->type)
    {
    case UART_RX_RDY:
        // push received data to ring buffer
        if (evt->data.rx.len > 0)
        {
            written = ring_buf_put(
                &gnss_ringbuf,
                &evt->data.rx.buf[evt->data.rx.offset],
                evt->data.rx.len
            );

            if (written < evt->data.rx.len)
            {
                LOG_WRN("Ring buffer overflow, lost %u bytes", evt->data.rx.len - written);
            }

            bytes_received += written;
            k_sem_give(&gnss_sem);
        }
        break;
    case UART_RX_BUF_REQUEST:
        // provide next buffer
        if (evt->data.rx.buf == uart_rx_buffer_a)
        {
            ret = uart_rx_buf_rsp(dev, uart_rx_buffer_b, DMA_BUFFER_SIZE);
            if (ret != 0)
            {
                LOG_ERR("Failed to provide next UART RX buffer: %d", ret);
            }
        }
        else
        {
            ret = uart_rx_buf_rsp(dev, uart_rx_buffer_a, DMA_BUFFER_SIZE);
            if (ret != 0)
            {
                LOG_ERR("Failed to provide next UART RX buffer: %d", ret);
            }
        }
        break;
    case UART_RX_BUF_RELEASED:
        break;
    case UART_RX_STOPPED:
        // restart uart rx
        ret = uart_rx_enable(dev, uart_rx_buffer_a, DMA_BUFFER_SIZE, SYS_FOREVER_MS);
        if (ret != 0)
        {
            LOG_ERR("Failed to restart UART RX: %d", ret);
        }
        break;
    default:
        break;
    }
}

int main(void)
{
    LOG_INF("=== UART GPS Demo (DMA) ===");
    LOG_INF("Expecting Ublox M8 on UART1 (GPIO4/5) at 9600 baud");

    timing_init();  // Initialize the timing subsystem
    timing_start(); // Start timing measurement

    if (!device_is_ready(gnss_uart_dev))
    {
        LOG_ERR("GNSS UART device not ready");
        return -1;
    }

    // setup UART DMA RX (async API)
    int ret = uart_callback_set(gnss_uart_dev, uart_async_callback, NULL);
    if (ret != 0)
    {
        LOG_ERR("Failed to set UART callback: %d", ret);
        return -1;
    }

    ret = uart_rx_enable(gnss_uart_dev, uart_rx_buffer_a, DMA_BUFFER_SIZE, SYS_FOREVER_MS);
    if (ret != 0)
    {
        LOG_ERR("Failed to enable UART RX: %d", ret);
        return -1;
    }

    while (1)
    {
        k_sleep(K_SECONDS(5));
        LOG_INF("Bytes received: %u, Sentences parsed: %u", bytes_received, sentence_parsed);
    }
    return 0;
}