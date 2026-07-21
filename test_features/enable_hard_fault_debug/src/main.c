#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(test_logging, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(thread_stack_area, 1024);
struct k_thread thread_state;

void trigger_hard_fault(void)
{
    k_sleep(K_MSEC(1000));  // Sleep for 1 second before triggering the fault

    // Trigger a hard fault by dereferencing a NULL pointer
    volatile int *ptr = NULL;
    *ptr = 42;  // This will cause a hard fault
}

void thread_func(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Thread started, will trigger hard fault in 1 second");
    k_sleep(K_MSEC(1000));

    // trigger hard fault
    trigger_hard_fault();
}

int main(void)
{
    k_thread_create(&thread_state, thread_stack_area, K_THREAD_STACK_SIZEOF(thread_stack_area),
                    thread_func, NULL, NULL, NULL,
                    7, 0, K_NO_WAIT);
    k_thread_name_set(&thread_state, "fault_thread");

    return 0;
}