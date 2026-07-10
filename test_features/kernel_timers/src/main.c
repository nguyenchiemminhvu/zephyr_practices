#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static struct k_timer watchdog_timer;
static struct k_timer single_shot_timer;

void watchdog_timer_handler(struct k_timer *timer_id)
{
    printk("Watchdog timer expired!\n");
}

void single_shot_timer_handler(struct k_timer *timer_id)
{
    printk("Single-shot timer expired!\n");
}

void timer_stop_func(struct k_timer *timer_id)
{
    printk("Watchdog timer stopped!\n");
}

int main(void)
{
    k_timer_init(&watchdog_timer, watchdog_timer_handler, timer_stop_func);
    k_timer_start(&watchdog_timer, K_SECONDS(2), K_SECONDS(2));
    k_timer_init(&single_shot_timer, single_shot_timer_handler, NULL);
    k_timer_start(&single_shot_timer, K_SECONDS(3), K_NO_WAIT);

    for (int i = 0; i < 10; i++)
    {
        printk("Main loop iteration: %d\n", i);
        k_sleep(K_SECONDS(1));
    }

    k_timer_status_sync(&watchdog_timer);
    printk("Watchdog timer status: %d\n", k_timer_status_get(&watchdog_timer));

    k_timer_stop(&watchdog_timer);

    return 0;
}