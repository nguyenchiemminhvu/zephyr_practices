/*
 * @file main.c
 * @brief Main file for the UART echo bot application.
 *
 * This application initializes the UART driver and echoes back any received data.
 *
 * Target build: qemu_cortex_a72
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

#define MSG_SIZE 64

struct uart_context
{
    char buffer[MSG_SIZE];
    size_t length;
};

// Define a message queue for UART messages, with a size of MSG_SIZE, a maximum of 10 messages, and an alignment of 4 bytes.
K_MSGQ_DEFINE(uart_msgq, sizeof(struct uart_context), 10, 4);

void uart_interrupt_callback(const struct device *dev, void *user_data)
{
    uart_irq_update(dev);
    if (!uart_irq_rx_ready(dev))
    {
        /* Not an RX event (e.g. TX ready interrupt) — nothing to do */
        return;
    }

    /*
     * Static buffer persists across interrupt calls.
     * QEMU fires one interrupt per character in raw mode, so we must
     * accumulate bytes here across multiple interrupts until a newline arrives.
     * k_malloc must NOT be used here — it allocates a fresh zeroed buffer
     * each interrupt and frees it at the end, discarding all accumulated data.
     */
    static struct uart_context rx_ctx = {0};

    uint8_t byte;
    while (uart_fifo_read(dev, &byte, 1) > 0)
    {
        if (byte == '\n' || byte == '\r')
        {
            if (rx_ctx.length == 0)
            {
                continue; /* ignore bare newlines with no content */
            }
            rx_ctx.buffer[rx_ctx.length] = '\0'; /* null-terminate */
            if (k_msgq_put(&uart_msgq, &rx_ctx, K_NO_WAIT) != 0)
            {
                /* UART message queue full, dropping message */
            }
            rx_ctx.length = 0; /* reset for next message */
        }
        else if (rx_ctx.length < MSG_SIZE - 1)
        {
            rx_ctx.buffer[rx_ctx.length++] = byte;
        }
        else
        {
            /* UART message too long, dropping message */
            rx_ctx.length = 0;
        }
    }
}

void uart_respond(const char *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        uart_poll_out(uart_dev, data[i]);
    }

    /* Must send '\r' and '\n' as two separate calls.
     * '\r\n' is a multi-character literal — NOT a string — and gets
     * silently truncated to a single byte ('\n'), dropping the carriage return. */
    uart_poll_out(uart_dev, '\r');
    uart_poll_out(uart_dev, '\n');
}

int main(void)
{
    if (!device_is_ready(uart_dev))
    {
        /* UART device not ready */
        return -1;
    }

    /* UART device ready */
    if (uart_irq_callback_user_data_set(uart_dev, uart_interrupt_callback, NULL) != 0)
    {
        /* Failed to set UART interrupt callback */
        return -1;
    }

    uart_irq_rx_enable(uart_dev);
    /* TX is handled via uart_poll_out (polling mode) — do NOT enable TX IRQ */
    /* uart_irq_tx_enable would flood the system with unhandled TX-ready interrupts */

    uart_respond("Hello! I'm your echo bot.", 24);
    uart_respond("Tell me something and press enter:", 33);

    struct uart_context msg = {0};

    while (k_msgq_get(&uart_msgq, &msg, K_FOREVER) == 0)
    {
        size_t msg_len = msg.length;
        uart_respond(msg.buffer, msg_len);
        memset(&msg, 0, sizeof(struct uart_context));
    }

    return 0;
}
