#include <zephyr/kernel.h>

#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY 5

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_STATIC_THREAD_TEST)

void static_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1)
    {
        printk("Static thread is running\n");
        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_DEFINE(thread_id, THREAD_STACK_SIZE, static_thread_entry, NULL, NULL, NULL, THREAD_PRIORITY, 0, -1);

#endif // CONFIG_ENABLE_THREADS_TEST && CONFIG_STATIC_THREAD_TEST

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_DYNAMIC_THREAD_TEST)

struct thread_result_data
{
    int return_code;
    bool is_completed;
};

K_THREAD_STACK_DEFINE(dynamic_thread_stack, THREAD_STACK_SIZE);
struct k_thread dynamic_thread;

void dynamic_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    struct thread_result_data *result = (struct thread_result_data *)p1;
    for (int i = 0; i < 3; ++i)
    {
        printk("Dynamic thread is running\n");
        result->return_code += 1;
        result->is_completed = true;
        k_sleep(K_MSEC(1000));
    }
}
#endif // CONFIG_ENABLE_THREADS_TEST && CONFIG_DYNAMIC_THREAD_TEST

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_THREAD_SUSPEND_RESUME_TEST)
K_THREAD_STACK_DEFINE(suspend_resume_thread_stack, THREAD_STACK_SIZE);
struct k_thread suspend_resume_thread;

void suspend_resume_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1)
    {
        printk("Suspend/Resume thread is running\n");
        k_sleep(K_MSEC(1000));
    }
}
#endif // CONFIG_ENABLE_THREADS_TEST && CONFIG_THREAD_SUSPEND_RESUME_TEST

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_THREAD_PREEMTION_TEST)
#define LOW_PRIORITY_STACK_SIZE 2048
#define HIGH_PRIORITY_STACK_SIZE 2048
#define LOW_PRIORITY 10
#define HIGH_PRIORITY 2

K_THREAD_STACK_DEFINE(low_priority_stack, LOW_PRIORITY_STACK_SIZE);
K_THREAD_STACK_DEFINE(high_priority_stack, HIGH_PRIORITY_STACK_SIZE);
struct k_thread low_priority_thread;
struct k_thread high_priority_thread;

void low_priority_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Low priority thread is running\n");
    k_busy_wait(3000000); // Simulate some work
    printk("Low priority thread completed its work\n");

    while (1)
    {
        printk("Low priority thread waiting for abort\n");
        k_sleep(K_MSEC(1000));
    }
}

void high_priority_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("High priority thread is running\n");
    k_busy_wait(1000000); // Simulate some work
    printk("High priority thread completed its work\n");

    while (1)
    {
        printk("High priority thread waiting for abort\n");
        k_sleep(K_MSEC(1000));
    }
}

#endif // CONFIG_ENABLE_THREADS_TEST && CONFIG_THREAD_PREEMTION_TEST

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_THREAD_YIELDING_TEST)
K_THREAD_STACK_DEFINE(thread1_stack, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(thread2_stack, THREAD_STACK_SIZE);
struct k_thread thread1;
struct k_thread thread2;

void thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1)
    {
        printk("Thread %p is running\n", k_current_get());
        k_busy_wait(500000); // Simulate some work
        k_yield();
    }
}

#endif // CONFIG_ENABLE_THREADS_TEST && CONFIG_THREAD_YIELDING_TEST

int main(void)
{
#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_STATIC_THREAD_TEST)
    printk("Running static thread demo after 1 second\n");
    k_sleep(K_MSEC(1000));
    k_thread_start(thread_id);
#endif

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_DYNAMIC_THREAD_TEST)
    printk("Running dynamic thread demo after 1 second\n");
    struct thread_result_data dynamic_thread_result = {0, false};
    k_tid_t thread_id = k_thread_create(
        &dynamic_thread,
        dynamic_thread_stack,
        K_THREAD_STACK_SIZEOF(dynamic_thread_stack),
        dynamic_thread_entry,
        &dynamic_thread_result, NULL, NULL,
        THREAD_PRIORITY,
        0,
        K_MSEC(10)
    );

    k_sleep(K_MSEC(3000)); // Wait for the thread to run for a while
    int join_rt = k_thread_join(thread_id, K_FOREVER);
    if (join_rt == 0)
    {
        printk("Thread joined successfully with return code: %d\n", dynamic_thread_result.return_code);
    }
    else
    {
        printk("Thread join failed with return code: %d\n", join_rt);
    }
#endif

#if defined(CONFIG_ENABLE_THREADS_TEST) && (defined(CONFIG_STATIC_THREAD_TEST) || defined(CONFIG_DYNAMIC_THREAD_TEST))
    k_sleep(K_MSEC(5000));
    k_thread_abort(thread_id);
#endif

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_THREAD_SUSPEND_RESUME_TEST)
    printk("Running thread suspend/resume demo\n");

    k_tid_t suspend_resume_thread_id = k_thread_create(
        &suspend_resume_thread,
        suspend_resume_thread_stack,
        K_THREAD_STACK_SIZEOF(suspend_resume_thread_stack),
        suspend_resume_thread_entry,
        NULL, NULL, NULL,
        THREAD_PRIORITY,
        0,
        K_NO_WAIT
    );

    bool thread_running = true;
    while (1)
    {
        if (thread_running)
        {
            printk("Suspending thread\n");
            k_thread_suspend(suspend_resume_thread_id);
            thread_running = false;
        }
        else
        {
            printk("Resuming thread\n");
            k_thread_resume(suspend_resume_thread_id);
            thread_running = true;
        }
        k_sleep(K_MSEC(3000));
    }
#endif

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_THREAD_PREEMTION_TEST)
    printk("Running thread preemption demo\n");
    k_tid_t low_priority_thread_id = k_thread_create(
        &low_priority_thread,
        low_priority_stack,
        K_THREAD_STACK_SIZEOF(low_priority_stack),
        low_priority_thread_entry,
        NULL, NULL, NULL,
        LOW_PRIORITY,
        0,
        K_MSEC(100)
    );

    k_tid_t high_priority_thread_id = k_thread_create(
        &high_priority_thread,
        high_priority_stack,
        K_THREAD_STACK_SIZEOF(high_priority_stack),
        high_priority_thread_entry,
        NULL, NULL, NULL,
        HIGH_PRIORITY,
        0,
        K_NO_WAIT
    );

    k_sleep(K_MSEC(5000)); // Wait for threads to complete
    k_thread_abort(low_priority_thread_id);
    k_thread_abort(high_priority_thread_id);
    printk("Thread preemption demo completed\n");
#endif

#if defined(CONFIG_ENABLE_THREADS_TEST) && defined(CONFIG_THREAD_YIELDING_TEST)
    printk("Running thread yielding demo\n");

    k_tid_t thread1_id = k_thread_create(
        &thread1,
        thread1_stack,
        K_THREAD_STACK_SIZEOF(thread1_stack),
        thread_entry,
        NULL, NULL, NULL,
        THREAD_PRIORITY,
        0,
        K_NO_WAIT
    );

    k_tid_t thread2_id = k_thread_create(
        &thread2,
        thread2_stack,
        K_THREAD_STACK_SIZEOF(thread2_stack),
        thread_entry,
        NULL, NULL, NULL,
        THREAD_PRIORITY,
        0,
        K_NO_WAIT
    );

    k_sleep(K_MSEC(7000)); // Let the threads run for a while
    k_thread_abort(thread1_id);
    k_thread_abort(thread2_id);
    printk("Thread yielding demo completed\n");
#endif

    return 0;
}