#include <zephyr/kernel.h>
#define THREAD_COUNT 10
#define STACK_SIZE 1024

struct k_sem toilet_semaphore;

void use_toilet_thread_func(void *p1, void *p2, void *p3)
{
    const char* thread_name = (const char*)p1;
    printk("%s gonna use the toilet\n", thread_name);
    k_sem_take(&toilet_semaphore, K_FOREVER); // Wait for a booth
    printk("%s is using the toilet\n", thread_name);
    k_sleep(K_SECONDS(2)); // Simulate time taken to use the toilet
    printk("%s is done using the toilet\n", thread_name);
    k_sem_give(&toilet_semaphore); // Release the booth
}

K_THREAD_STACK_ARRAY_DEFINE(stacks, THREAD_COUNT, STACK_SIZE);
static struct k_thread threads[THREAD_COUNT];

int main(void)
{
    printk("Test features: kernel semaphores\n");
    printk("Toilet example: multiple booths open for use\n");

    k_sem_init(&toilet_semaphore, 5, 5); // Initialize semaphore with 5 available booths

    k_tid_t tids[THREAD_COUNT];
    char names[THREAD_COUNT][20];
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        snprintf(names[i], 20, "Thread %d", i + 1);
        tids[i] = k_thread_create(&threads[i], stacks[i],
                                  K_THREAD_STACK_SIZEOF(stacks[i]),
                                  use_toilet_thread_func, names[i], NULL, NULL,
                                  K_PRIO_PREEMPT(1), 0, K_NO_WAIT);
    }

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        k_thread_join(tids[i], K_FOREVER); // Wait for all threads to finish
    }

    printk("All threads have finished using the toilet.\n");
    return 0;
}