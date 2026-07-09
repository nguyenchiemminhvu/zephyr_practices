#include <zephyr/kernel.h>

#define STACK_SIZE 1024
#define PRIORITY K_PRIO_PREEMPT(1)

K_THREAD_STACK_DEFINE(producer_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(consumer_stack_1, STACK_SIZE);
K_THREAD_STACK_DEFINE(consumer_stack_2, STACK_SIZE);

struct k_condvar condvar;
struct k_mutex mutex;

#define BUFFER_SIZE 10
int buffer[BUFFER_SIZE];
int count = 0;

void producer_thread_func(void *arg1, void *arg2, void *arg3)
{
    while (1)
    {
        k_mutex_lock(&mutex, K_FOREVER);
        while (count != 0)
        {
            k_condvar_wait(&condvar, &mutex, K_FOREVER);
        }

        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            buffer[i] = i;
        }
        count = BUFFER_SIZE;
        k_sleep(K_MSEC(1000)); // Simulate some work being done
        printk("Produced %d items\n", count);
        k_condvar_broadcast(&condvar);
        k_mutex_unlock(&mutex);
    }
}

void consumer_thread_func(void *arg1, void *arg2, void *arg3)
{
    int id = (int)(intptr_t)arg1;
    while (1)
    {
        k_mutex_lock(&mutex, K_FOREVER);
        while (count == 0)
        {
            k_condvar_wait(&condvar, &mutex, K_FOREVER);
        }

        count -= 1;
        printk("Consumer %d: Consumed 1 item, %d items left\n", id, count);

        if (count == 0)
        {
            k_condvar_broadcast(&condvar);
        }
        k_mutex_unlock(&mutex);
        k_sleep(K_MSEC(100)); // Remove from ready queue for one tick so the other consumer can run
    }
}

int main(void)
{
    printk("Test features: kernel conditional variables\n");
    printk("Example: producer and consumer\n");

    k_condvar_init(&condvar);
    k_mutex_init(&mutex);

    struct k_thread producer_thread;
    k_thread_create(
        &producer_thread, 
        producer_stack, 
        K_THREAD_STACK_SIZEOF(producer_stack), 
        producer_thread_func, 
        NULL, NULL, NULL, 
        PRIORITY, 0, K_NO_WAIT
    );

    struct k_thread consumer_thread_1;
    k_thread_create(
        &consumer_thread_1, 
        consumer_stack_1, 
        K_THREAD_STACK_SIZEOF(consumer_stack_1), 
        consumer_thread_func, 
        (void *)1, NULL, NULL, 
        PRIORITY, 0, K_NO_WAIT
    );

    struct k_thread consumer_thread_2;
    k_thread_create(
        &consumer_thread_2, 
        consumer_stack_2, 
        K_THREAD_STACK_SIZEOF(consumer_stack_2), 
        consumer_thread_func, 
        (void *)2, NULL, NULL, 
        PRIORITY, 0, K_NO_WAIT
    );

    return 0;
}