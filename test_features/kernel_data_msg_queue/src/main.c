#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct queue_item
{
    uint32_t field1;
    uint32_t field2;
    uint32_t field3;
};

void producer_thread_func(void *arg1, void *arg2, void *arg3)
{
    struct k_msgq *msgq = (struct k_msgq *)arg1;

    while (1)
    {
        struct queue_item item;
        int ret = k_msgq_get(msgq, &item, K_FOREVER);
        if (ret == 0)
        {
            printk("Consumed: field1=%u\n", item.field1);
        }
        else
        {
            printk("Queue empty, unable to consume item\n");
            k_sleep(K_MSEC(10)); // Sleep for a short duration before retrying
            if (k_msgq_peek(msgq, &item) == 0)
            {
                printk("Peeked: field1=%u\n", item.field1);
            }
            else
            {
                printk("Queue still empty after peek\n");
            }
        }

        k_sleep(K_MSEC(200));
    }
}

void consumer_thread_func(void *arg1, void *arg2, void *arg3)
{
    struct k_msgq *msgq = (struct k_msgq *)arg1;
    
    int32_t ticks = 0;
    while (1)
    {
        struct queue_item item;
        item.field1 = ticks++;
        int ret = k_msgq_put(msgq, &item, K_NO_WAIT);
        if (ret == 0)
        {
            printk("Produced: field1=%u\n", item.field1);
        }
        else
        {
            printk("Queue full, unable to produce item\n");
        }

        k_sleep(K_MSEC(100));
    }
}

#define THREAD_STACK_SIZE 1024
K_THREAD_STACK_DEFINE(producer_stack, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(consumer_stack, THREAD_STACK_SIZE);

struct k_thread producer_thread_data;
struct k_thread consumer_thread_data;

int main(void)
{
    struct queue_item *buffer = (struct queue_item *)k_malloc(sizeof(struct queue_item) * 10);
    struct k_msgq q;
    k_msgq_init(&q, (char*)buffer, sizeof(struct queue_item), 10);

    k_tid_t producer_tid = k_thread_create(&producer_thread_data, producer_stack, THREAD_STACK_SIZE,
                                            producer_thread_func, &q, NULL, NULL,
                                            1, 0, K_NO_WAIT);
    k_tid_t consumer_tid = k_thread_create(&consumer_thread_data, consumer_stack, THREAD_STACK_SIZE,
                                            consumer_thread_func, &q, NULL, NULL,
                                            1, 0, K_NO_WAIT);

    k_sleep(K_FOREVER); // Keep the main thread alive to allow producer and consumer threads to run
    return 0;
}