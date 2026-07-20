#include <zephyr/kernel.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dt_demo, LOG_LEVEL_DBG);

// #define ONBOARD_LED_NODE DT_NODELABEL(led0)
#define ONBOARD_LED_NODE DT_ALIAS(onboard_led)

// #define GNSS_UART_NODE DT_NODELABEL(uart1)
#define GNSS_UART_NODE DT_ALIAS(gnss_uart)
#define GNSS_UART_BAUD DT_PROP(GNSS_UART_NODE, current-speed)

int main(void)
{
    LOG_INF("Hello from Zephyr! %s", CONFIG_BOARD);

    static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(ONBOARD_LED_NODE, gpios);
    while (!device_is_ready(led.port))
    {
        LOG_ERR("LED device not ready");
        k_sleep(K_MSEC(1000));
    }

    LOG_INF("LED device is ready %s (pin %d)", led.port->name, led.pin);

    int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure LED pin");
        return ret;
    }

    const struct device *gnss_uart_dev = DEVICE_DT_GET(GNSS_UART_NODE);
    while (!device_is_ready(gnss_uart_dev))
    {
        LOG_ERR("GNSS UART device not ready");
        k_sleep(K_MSEC(1000));
    }

    LOG_INF("GNSS UART device is ready %s", gnss_uart_dev->name);

    bool led_state = false;
    while (1)
    {
        gpio_pin_toggle_dt(&led);
        LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
        led_state = !led_state;
        k_sleep(K_MSEC(500));
    }

    return 0;
}
