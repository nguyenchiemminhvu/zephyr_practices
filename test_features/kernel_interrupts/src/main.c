#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define DEBOUNCE_TIMEOUT_MS K_MSEC(50)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw_isr), gpios);
static const struct gpio_dt_spec led    = GPIO_DT_SPEC_GET(DT_ALIAS(led_isr), gpios);

static struct gpio_callback button_cb_data;
static struct k_work debounce_work;

static void debounce_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    if (gpio_pin_get_dt(&button) == 1)
    {
        LOG_INF("Valid Button Interrupt Confirmed. Toggling LED State.");
        gpio_pin_toggle_dt(&led);
    }
}

static void button_pressed_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    k_work_submit(&debounce_work);
}

int main(void)
{
    int ret;

    LOG_INF("Initializing Raspberry Pi Pico ISR Demo App...");

    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button))
    {
        LOG_ERR("Error: Hardware GPIO devices are not ready.");
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure LED pin: %d", ret);
        return ret;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure Button pin: %d", ret);
        return ret;
    }

    k_work_init(&debounce_work, debounce_work_handler);

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure Interrupt on Button pin: %d", ret);
        return ret;
    }

    gpio_init_callback(&button_cb_data, button_pressed_isr, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret < 0)
    {
        LOG_ERR("Failed to append runtime GPIO driver callback: %d", ret);
        return ret;
    }

    LOG_INF("System Configuration complete. Awaiting edge-triggered ISR requests.");

    while (1)
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}
