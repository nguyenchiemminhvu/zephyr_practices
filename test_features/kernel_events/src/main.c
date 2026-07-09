#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 1024
#define PRIORITY 5
#define MAX_THREADS 5
K_THREAD_STACK_ARRAY_DEFINE(thread_stacks, MAX_THREADS, STACK_SIZE);
static struct k_thread threads[MAX_THREADS];

static struct k_event global_event;

#if defined(CONFIG_ONE_SRC_MANY_MONITORS_NO_CLEAR_BIT)
void event_monitor_thread_func(void *p1, void *p2, void *p3)
{
    uint32_t now_event_bits = *(uint32_t *)p1;
    uint32_t deferred_event_bits = *(uint32_t *)p2;
    uint32_t periodical_event_bits = *(uint32_t *)p3;

    if (k_event_wait(&global_event, now_event_bits, false, K_MSEC(500)) == now_event_bits)
    {
        printk("Thread %p received now_event_bits: 0x%08X\n", k_current_get(), now_event_bits);
    }

    if (k_event_wait(&global_event, deferred_event_bits, false, K_MSEC(2000)) == deferred_event_bits)
    {
        printk("Thread %p received deferred_event_bits: 0x%08X\n", k_current_get(), deferred_event_bits);
    }

    while (1)
    {
        if (k_event_wait(&global_event, periodical_event_bits, false, K_FOREVER) == periodical_event_bits)
        {
            printk("Thread %p received periodical_event_bits: 0x%08X\n", k_current_get(), periodical_event_bits);
        }

        k_sleep(K_MSEC(1000));
    }
}
#endif

#if defined(CONFIG_ONE_SRC_MANY_MONITORS_WITH_CLEAR_BIT)
void event_monitor_thread_func(void *p1, void *p2, void *p3)
{
    uint32_t now_event_bits = *(uint32_t *)p1;
    uint32_t deferred_event_bits = *(uint32_t *)p2;
    uint32_t periodical_event_bits = *(uint32_t *)p3;

    if (k_event_wait(&global_event, now_event_bits, true, K_MSEC(500)) == now_event_bits)
    {
        printk("Thread %p received now_event_bits: 0x%08X\n", k_current_get(), now_event_bits);
    }

    if (k_event_wait(&global_event, deferred_event_bits, true, K_MSEC(2000)) == deferred_event_bits)
    {
        printk("Thread %p received deferred_event_bits: 0x%08X\n", k_current_get(), deferred_event_bits);
    }

    while (1)
    {
        if (k_event_wait(&global_event, periodical_event_bits, true, K_FOREVER) == periodical_event_bits)
        {
            printk("Thread %p received periodical_event_bits: 0x%08X\n", k_current_get(), periodical_event_bits);
        }

        k_sleep(K_MSEC(1000));
    }
}
#endif

void test_wait_any_logic(void)
{
#if defined(CONFIG_WAIT_ANY_LOGIC)
    printk("Test set and wait in one thread is enabled.\n");

    k_event_init(&global_event);
    if (k_event_wait(&global_event, 0x01, false, K_MSEC(1000)) == 0)
    {
        printk("Event wait timed out as expected.\n");
    }
    else
    {
        printk("Event wait did not time out as expected.\n");
    }

    k_event_set(&global_event, 0x01);
    if (k_event_wait(&global_event, 0x01, false, K_MSEC(1000)) == 0x01)
    {
        printk("Event wait succeeded as expected.\n");
    }
    else
    {
        printk("Event wait did not succeed as expected.\n");
    }

    k_event_clear(&global_event, 0x02);
    if (k_event_wait(&global_event, 0x01, false, K_MSEC(1000)) == 0x01)
    {
        printk("Event wait succeeded as expected after clearing unrelated bits.\n");
    }
    else
    {
        printk("Event wait did not succeed as expected after clearing unrelated bits.\n");
    }

    k_event_clear(&global_event, 0xFFFFFFFF);
    if (k_event_wait(&global_event, 0x01, false, K_MSEC(1000)) == 0)
    {
        printk("Event wait timed out as expected after clearing all bits.\n");
    }
    else
    {
        printk("Event wait did not time out as expected after clearing all bits.\n");
    }

    k_event_set(&global_event, 0x01);
    for (int i = 0; i < 5; i++)
    {
        if (k_event_wait(&global_event, 0x01, false, K_MSEC(1000)) == 0x01)
        {
            printk("Event wait succeeded as expected in iteration %d.\n", i);
        }
        else
        {
            printk("Event wait did not succeed as expected in iteration %d.\n", i);
        }
    }
#endif // defined(CONFIG_WAIT_ANY_LOGIC)
}

void test_wait_all_logic(void)
{
#if defined(CONFIG_WAIT_ALL_LOGIC)
    printk("Test wait all logic is enabled.\n");

    k_event_init(&global_event);

    k_event_post(&global_event, 0b0001);
    if (k_event_wait_all(&global_event, 0b1111, false, K_MSEC(1000)) == 0)
    {
        printk("Event wait all timed out as expected.\n");
    }
    else
    {
        printk("Event wait all did not time out as expected.\n");
    }

    k_event_post(&global_event, 0b0010);
    if (k_event_wait_all(&global_event, 0b1111, false, K_MSEC(1000)) == 0)
    {
        printk("Event wait all timed out as expected.\n");
    }
    else
    {
        printk("Event wait all did not time out as expected.\n");
    }

    k_event_post(&global_event, 0b0100);
    if (k_event_wait_all(&global_event, 0b1111, false, K_MSEC(1000)) == 0)
    {
        printk("Event wait all timed out as expected.\n");
    }
    else
    {
        printk("Event wait all did not time out as expected.\n");
    }

    k_event_post(&global_event, 0b1000);
    if (k_event_wait_all(&global_event, 0b1111, false, K_MSEC(1000)) == 0b1111)
    {
        printk("Event wait all succeeded as expected.\n");
    }
    else
    {
        printk("Event wait all did not succeed as expected.\n");
    }

    k_event_set(&global_event, 0xFFFFFFFF);
    for (int i = 0; i < 5; i++)
    {
        if (k_event_wait_all(&global_event, 0b1111, false, K_MSEC(1000)) == 0b1111)
        {
            printk("Event wait all succeeded as expected in iteration %d.\n", i);
        }
        else
        {
            printk("Event wait all did not succeed as expected in iteration %d.\n", i);
        }
    }
#endif // defined(CONFIG_WAIT_ALL_LOGIC)
}

void test_one_src_many_monitors_no_clear_bit(void)
{
#if defined(CONFIG_ONE_SRC_MANY_MONITORS_NO_CLEAR_BIT)
    printk("Test one source many monitors no clear bit is enabled.\n");

    uint32_t now_event_bits = 0b0001;
    uint32_t deferred_event_bits = 0b0010;
    uint32_t periodical_event_bits = 0b0100;

    for (int i = 0; i < MAX_THREADS; i++)
    {
        k_thread_create(&threads[i], thread_stacks[i], STACK_SIZE,
                        event_monitor_thread_func,
                        &now_event_bits, &deferred_event_bits, &periodical_event_bits,
                        PRIORITY, 0, K_NO_WAIT);
    }

    k_event_init(&global_event);

    k_event_set(&global_event, now_event_bits);
    
    k_sleep(K_MSEC(1000));
    k_event_set(&global_event, deferred_event_bits);

    k_event_set(&global_event, 0);
    while (1)
    {
        k_event_set(&global_event, periodical_event_bits);
        k_sleep(K_MSEC(1000));
    }
#endif // defined(CONFIG_ONE_SRC_MANY_MONITORS_NO_CLEAR_BIT)
}

void test_one_src_many_monitors_with_clear_bit(void)
{
#if defined(CONFIG_ONE_SRC_MANY_MONITORS_WITH_CLEAR_BIT)
    printk("Test one source many monitors with clear bit is enabled.\n");

    uint32_t now_event_bits = 0b0001;
    uint32_t deferred_event_bits = 0b0010;
    uint32_t periodical_event_bits = 0b0100;

    for (int i = 0; i < MAX_THREADS; i++)
    {
        k_thread_create(&threads[i], thread_stacks[i], STACK_SIZE,
                        event_monitor_thread_func,
                        &now_event_bits, &deferred_event_bits, &periodical_event_bits,
                        PRIORITY, 0, K_NO_WAIT);
    }

    k_event_init(&global_event);

    k_event_set(&global_event, now_event_bits);
    
    k_sleep(K_MSEC(1000));
    k_event_set(&global_event, deferred_event_bits);

    k_event_set(&global_event, 0);
    while (1)
    {
        k_event_set(&global_event, periodical_event_bits);
        k_sleep(K_MSEC(1000));
    }
#endif // defined(CONFIG_ONE_SRC_MANY_MONITORS_WITH_CLEAR_BIT)
}

int main(void)
{
    test_wait_any_logic();
    test_wait_all_logic();
    test_one_src_many_monitors_no_clear_bit();
    test_one_src_many_monitors_with_clear_bit();
    return 0;
}