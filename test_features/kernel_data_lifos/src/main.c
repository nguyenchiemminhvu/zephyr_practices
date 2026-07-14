#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct data_item
{
    void *fifo_reserved; /* 1st word reserved for use by lifo */
    int data;
};

K_FIFO_DEFINE(global_st);

void consumer_thread(void *p1, void *p2, void *p3)
{
    while (1)
    {
        struct data_item *item = k_fifo_get(&global_st, K_NO_WAIT);
        if (item)
        {
            printk("Global LIFO: %d\n", item->data);
            k_free(item);
        }
        else
        {
            k_sleep(K_MSEC(500));
        }
    }
}
K_THREAD_DEFINE(consumer_tid, 1024, consumer_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    struct k_lifo local_st;
    k_lifo_init(&local_st);
    int32_t test_local_data[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < sizeof(test_local_data) / sizeof(test_local_data[0]); i++)
    {
        struct data_item *item = (struct data_item *)k_malloc(sizeof(struct data_item));
        item->data = test_local_data[i];
        k_lifo_put(&local_st, item);
    }

    for (int i = 0; i < sizeof(test_local_data) / sizeof(test_local_data[0]); i++)
    {
        struct data_item *item = k_lifo_get(&local_st, K_FOREVER);
        printk("Local LIFO: %d\n", item->data);
        k_free(item);
    }

    int count = 0;
    while (1)
    {
        struct data_item *item = (struct data_item *)k_malloc(sizeof(struct data_item));
        item->data = count++;
        k_lifo_put(&global_st, item);
        k_sleep(K_MSEC(100));
    }

    return 0;
}