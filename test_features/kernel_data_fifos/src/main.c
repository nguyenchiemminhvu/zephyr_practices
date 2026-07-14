#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct data_item
{
    void *fifo_reserved; /* 1st word reserved for use by fifo */
    int data;
};

K_FIFO_DEFINE(global_q);

void consumer_thread(void *p1, void *p2, void *p3)
{
    while (1)
    {
        struct data_item *item = k_fifo_get(&global_q, K_FOREVER);
        printk("Global FIFO: %d\n", item->data);
        k_free(item);
    }
}

K_THREAD_DEFINE(consumer_tid, 1024, consumer_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    struct k_fifo local_q;
    k_fifo_init(&local_q);
    int32_t test_local_data[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < sizeof(test_local_data) / sizeof(test_local_data[0]); i++)
    {
        struct data_item *item = (struct data_item *)k_malloc(sizeof(struct data_item));
        item->data = test_local_data[i];
        k_fifo_put(&local_q, item);
    }

    for (int i = 0; i < sizeof(test_local_data) / sizeof(test_local_data[0]); i++)
    {
        struct data_item *item = k_fifo_get(&local_q, K_FOREVER);
        printk("Local FIFO: %d\n", item->data);
        k_free(item);
    }

    int count = 0;
    while (1)
    {
        struct data_item *item = (struct data_item *)k_malloc(sizeof(struct data_item));
        item->data = count++;
        k_fifo_put(&global_q, item);
        k_sleep(K_MSEC(1000));
    }

    return 0;
}