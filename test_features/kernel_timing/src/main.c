#include <zephyr/kernel.h>
#include <zephyr/timing/timing.h>

#include <stdint.h>

void performance_test(void)
{
    timing_init();  // Initialize the timing subsystem
    timing_start(); // Start timing measurement

    timing_t start_time, end_time;

    // Start timing
    start_time = timing_counter_get();

    // Perform the operation to be measured
    for (volatile int i = 0; i < 1000000; i++)
    {
        // Simulate some work
    }

    // End timing
    end_time = timing_counter_get();

    // Calculate elapsed time in nanoseconds
    uint32_t cycles = timing_cycles_get(&start_time, &end_time);
    uint32_t elapsed_time = timing_cycles_to_ns(cycles);

    timing_stop(); // Stop timing measurement

    printk("Elapsed time: %u milliseconds\n", elapsed_time / 1000000); // Print elapsed time in milliseconds
}

int main(void)
{
    printk("Starting performance test...\n");
    while (1)
    {
        k_sleep(K_SECONDS(1)); // Sleep for 1 second before running the test again
        performance_test();
    }
    printk("Performance test completed.\n");
    return 0;
}