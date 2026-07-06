#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <string.h>

#if CONFIG_ENABLE_STACK_OVERFLOW_TEST
__attribute__((noinline)) void force_stack_overflow(volatile int depth)
{
    volatile char heavy_buffer[256];
    heavy_buffer[0] = (char)depth;
    printk("Current recursion depth: %d\n", depth);
    force_stack_overflow(depth + 1);
    printk("%c", heavy_buffer[0]);
}
#endif // CONFIG_ENABLE_STACK_OVERFLOW_TEST

int main(void)
{
#if CONFIG_ENABLE_STACK_OVERFLOW_TEST
    printk("Starting stack overflow demonstration on QEMU Cortex-A72...\n");
    k_msleep(1000);

    force_stack_overflow(1); 
#endif // CONFIG_ENABLE_STACK_OVERFLOW_TEST


#if CONFIG_ENABLE_CRASH_TEST
    printk("Starting crash demonstration on QEMU Cortex-A72...\n");
    k_msleep(1000);
    int *ptr = NULL;
    *ptr = 42; // This will cause a crash (null pointer dereference)
#endif // CONFIG_ENABLE_CRASH_TEST

    printk("All tests completed. System is still running.\n");

    return 0;
}
