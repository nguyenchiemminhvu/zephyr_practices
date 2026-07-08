#include <zephyr/kernel.h>

#define THREAD_STACK_SIZE 2048
#define THREAD_PRIORITY 5

K_MUTEX_DEFINE(shared_mutex);
int shared_resource = 0;

void thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    for (volatile int i = 0; i < 10000; ++i)
    {
        k_mutex_lock(&shared_mutex, K_FOREVER);
        int temp = shared_resource;
        temp++;
        k_busy_wait(5); // Simulate some work
        shared_resource = temp;
        k_mutex_unlock(&shared_mutex);
    }
}

K_THREAD_DEFINE(thread_id_1, THREAD_STACK_SIZE, thread_entry, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread_id_2, THREAD_STACK_SIZE, thread_entry, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    printk("Synchronization primitives demo\n");

    k_thread_join(thread_id_1, K_FOREVER);
    k_thread_join(thread_id_2, K_FOREVER);
    printk("Final value of shared_resource: %d\n", shared_resource);

    return 0;
}