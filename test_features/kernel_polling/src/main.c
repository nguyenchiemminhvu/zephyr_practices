#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct k_sem sem1;
struct k_sem sem2;
struct k_fifo fifo1;

struct fifo_payload {
    void *data;
};

struct fifo_payload fifo_payload_instance = {
    .data = (void *)0xDEADBEEF,
};

struct k_poll_event poll_events[3] = {
    K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &sem1),
    K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &sem2),
    K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &fifo1),
};

void working_with_poll_thread_func(void *arg1, void *arg2, void *arg3)
{
    int ret;

    while (1)
    {
        int available_mask = 0;
        printk("Waiting for semaphores to become available...\n");
        ret = k_poll(poll_events, 3, K_FOREVER);
        if (ret == 0)
        {
            if (poll_events[0].state == K_POLL_STATE_SEM_AVAILABLE)
            {
                available_mask |= 0x01;
                k_sem_take(&sem1, K_NO_WAIT);
            }
            if (poll_events[1].state == K_POLL_STATE_SEM_AVAILABLE)
            {
                available_mask |= 0x02;
                k_sem_take(&sem2, K_NO_WAIT);
            }
            if (poll_events[2].state == K_POLL_STATE_FIFO_DATA_AVAILABLE)
            {
                available_mask |= 0x04;
                void *data = k_fifo_get(&fifo1, K_NO_WAIT);
                printk("Data received from FIFO: %p\n", data);
            }
        }

        printk("Available mask: 0x%03x\n", available_mask);
        poll_events[0].state = K_POLL_STATE_NOT_READY;
        poll_events[1].state = K_POLL_STATE_NOT_READY;
        poll_events[2].state = K_POLL_STATE_NOT_READY;
        if (available_mask != 0x007)
        {
            printk("Conditions are fulfilled, but maybe one of the semaphores was taken by another thread already\n");
        }
    }
}

void sem_processing_thread_func(void *arg1, void *arg2, void *arg3)
{
    while (1)
    {
        k_sleep(K_MSEC(1000));
        k_sem_give(&sem1);
        k_sem_give(&sem2);
        k_fifo_put(&fifo1, &fifo_payload_instance);
    }
}

#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY 7
K_THREAD_STACK_DEFINE(working_with_poll_thread_stack, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(sem_processing_thread_stack, THREAD_STACK_SIZE);

int main(void)
{
    printk("Kernel polling example started\n");

    k_sem_init(&sem1, 0, 1);
    k_sem_init(&sem2, 0, 1);
    k_fifo_init(&fifo1);

    k_thread_create(&(struct k_thread){}, working_with_poll_thread_stack, THREAD_STACK_SIZE,
                    working_with_poll_thread_func, NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&(struct k_thread){}, sem_processing_thread_stack, THREAD_STACK_SIZE,
                    sem_processing_thread_func, NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);

    return 0;
}
