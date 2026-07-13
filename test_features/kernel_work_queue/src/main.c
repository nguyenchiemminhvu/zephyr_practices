#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#if defined(CONFIG_WORK_QUEUE_POLL_TEST)
void basic_work_handler(struct k_work *work)
{
    printk("Basic work handler executed.\n");
}
#endif // CONFIG_WORK_QUEUE_POLL_TEST

#if defined(CONFIG_WORK_QUEUE_DELAY_TEST)
void delayed_work_handler(struct k_work *work)
{
    printk("Delayed work handler executed.\n");
}
#endif // CONFIG_WORK_QUEUE_DELAY_TEST

void test_basic_work_queue(void)
{
#if defined(CONFIG_BASIC_WORK_QUEUE_TEST)
    printk("Running basic work queue test...\n");
    
    struct k_work basic_work;
    k_work_init(&basic_work, basic_work_handler);

    int submit_result = k_work_submit(&basic_work);
    if (submit_result == 1)
    {
        printk("Basic work submitted successfully.\n");
    }
    else
    {
        printk("Failed to submit basic work. Error code: %d\n", submit_result);
    }

    k_sleep(K_MSEC(100)); // Allow some time for the work to be processed
#endif // CONFIG_BASIC_WORK_QUEUE_TEST
}

void test_work_queue_delay(void)
{
#if defined(CONFIG_WORK_QUEUE_DELAY_TEST)
    printk("Running work queue delay test...\n");
    
    struct k_work_delayable delayed_work;
    k_work_init_delayable(&delayed_work, delayed_work_handler);

    int submit_result = k_work_schedule(&delayed_work, K_MSEC(1000));
    if (submit_result == 1)
    {
        printk("Delayed work scheduled successfully.\n");
    }
    else
    {
        printk("Failed to schedule delayed work. Error code: %d\n", submit_result);
    }

    k_sleep(K_MSEC(1500)); // Allow some time for the delayed work to be processed
#endif // CONFIG_WORK_QUEUE_DELAY_TEST
}

#if defined(CONFIG_WORK_QUEUE_POLL_TEST)
struct k_sem sem;
struct k_fifo fifo;

struct fifo_packet
{
    void *fifo_reserved; // 1st word reserved for use by fifo
    int data;
};

#define TOTAL_EVENTS 2
static struct k_poll_event poll_events[TOTAL_EVENTS];

void poll_work_handler(struct k_work *work)
{
    printk("Poll work handler started.\n");

    if (poll_events[0].state == K_POLL_STATE_SEM_AVAILABLE)
    {
        printk("Semaphore is available.\n");
        k_sem_take(&sem, K_NO_WAIT);
        poll_events[0].state = K_POLL_STATE_NOT_READY; // Reset the state
    }

    if (poll_events[1].state == K_POLL_STATE_FIFO_DATA_AVAILABLE)
    {
        printk("FIFO data is available.\n");
        struct fifo_packet *packet = k_fifo_get(&fifo, K_NO_WAIT);
        if (packet)
        {
            printk("Received data from FIFO: %d\n", packet->data);
            k_free(packet); // Free the allocated memory
        }
        poll_events[1].state = K_POLL_STATE_NOT_READY; // Reset the state
    }

    printk("Poll work handler completed.\n");
}
#endif // CONFIG_WORK_QUEUE_POLL_TEST

void test_work_queue_poll(void)
{
#if defined(CONFIG_WORK_QUEUE_POLL_TEST)
    printk("Running work queue poll test...\n");

    k_sem_init(&sem, 0, 1);
    k_fifo_init(&fifo);
    
    k_poll_event_init(&poll_events[0], K_POLL_TYPE_SEM_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &sem);
    k_poll_event_init(&poll_events[1], K_POLL_TYPE_FIFO_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &fifo);

    struct k_work_poll poll_work;
    k_work_poll_init(&poll_work, poll_work_handler);

    int submit_result = k_work_poll_submit(&poll_work, poll_events, TOTAL_EVENTS, K_FOREVER);
    if (submit_result == 0)
    {
        printk("Poll work submitted successfully.\n");
    }
    else
    {
        printk("Failed to submit poll work. Error code: %d\n", submit_result);
    }

    k_sleep(K_MSEC(1000));
    // give a semaphore
    k_sem_give(&sem);

    submit_result = k_work_poll_submit(&poll_work, poll_events, TOTAL_EVENTS, K_FOREVER);
    if (submit_result == 0)
    {
        printk("Poll work submitted successfully after semaphore give.\n");
    }
    else
    {
        printk("Failed to submit poll work after semaphore give. Error code: %d\n", submit_result);
    }

    k_sleep(K_MSEC(1000));
    // make fifo data available
    struct fifo_packet *packet = (struct fifo_packet *)k_malloc(sizeof(struct fifo_packet));
    packet->data = 42;
    k_fifo_put(&fifo, packet);

    k_sleep(K_MSEC(1000));
    printk("Poll work test completed.\n");
#endif // CONFIG_WORK_QUEUE_POLL_TEST
}

#if defined(CONFIG_CONTAINER_OF_TEST)
struct work_context
{
    struct k_work work;
    const char* device_name;
    const char* driver_version;
};

void container_of_work_handler(struct k_work *work)
{
    printk("Container_of work handler executed.\n");

    struct work_context *context = CONTAINER_OF(work, struct work_context, work);
    printk("Device Name: %s\n", context->device_name);
    printk("Driver Version: %s\n", context->driver_version);
}
#endif // CONFIG_CONTAINER_OF_TEST

void test_container_of(void)
{
#if defined(CONFIG_CONTAINER_OF_TEST)
    printk("Running container_of test...\n");
    
    struct work_context context = {
        .device_name = "MyDevice",
        .driver_version = "1.0.0"
    };

    k_work_init(&context.work, container_of_work_handler);
    int submit_result = k_work_submit(&context.work);
    if (submit_result == 1)
    {
        printk("Container_of work submitted successfully.\n");
    }
    else
    {
        printk("Failed to submit container_of work. Error code: %d\n", submit_result);
    }

    k_sleep(K_MSEC(100)); // Allow some time for the work to be processed
#endif // CONFIG_CONTAINER_OF_TEST
}

#if defined(CONFIG_ADDITIONAL_WORK_QUEUE_TEST)
#define ADDITIONAL_WORK_QUEUE_STACK_SIZE 1024
K_THREAD_STACK_DEFINE(additional_work_queue_stack, ADDITIONAL_WORK_QUEUE_STACK_SIZE);
struct k_work_q additional_work_queue;

void additional_work_handler(struct k_work *work)
{
    printk("Additional work handler executed.\n");
}
#endif // CONFIG_ADDITIONAL_WORK_QUEUE_TEST

void test_additional_work_queue(void)
{
#if defined(CONFIG_ADDITIONAL_WORK_QUEUE_TEST)
    printk("Running additional work queue test...\n");

    k_work_queue_init(&additional_work_queue);

    struct k_work_queue_config config = {
        .name = "additional_work_queue",
        .no_yield = false,
    };

    k_work_queue_start(
        &additional_work_queue,
        additional_work_queue_stack,
        ADDITIONAL_WORK_QUEUE_STACK_SIZE,
        K_PRIO_PREEMPT(1),
        &config
    );

    struct k_work custom_work_1;
    k_work_init(&custom_work_1, additional_work_handler);
    k_work_submit_to_queue(&additional_work_queue, &custom_work_1);

    struct k_work custom_work_2;
    k_work_init(&custom_work_2, additional_work_handler);
    k_work_submit_to_queue(&additional_work_queue, &custom_work_2);

    k_sleep(K_MSEC(100)); // Allow some time for the work queue to start
#endif // CONFIG_ADDITIONAL_WORK_QUEUE_TEST
}

#if defined(CONFIG_WORK_QUEUE_SYNC_TEST)
void sync_work_handler(struct k_work *work)
{
    printk("Sync work handler executed.\n");
    k_sleep(K_MSEC(2000)); // Simulate some work being done
}
#endif // CONFIG_WORK_QUEUE_SYNC_TEST

void test_work_queue_sync(void)
{
#if defined(CONFIG_WORK_QUEUE_SYNC_TEST)
    printk("Running work queue sync test...\n");
    struct k_work sync_work;
    struct k_work_sync sync_state;
    k_work_init(&sync_work, sync_work_handler);
    int submit_result = k_work_submit(&sync_work);
    if (submit_result == 1)
    {
        printk("Sync work submitted successfully.\n");
    }
    else
    {
        printk("Failed to submit sync work. Error code: %d\n", submit_result);
    }

    k_work_flush(&sync_work, &sync_state); // Wait for all work to complete
    printk("Done with sync work test.\n");
#endif // CONFIG_WORK_QUEUE_SYNC_TEST
}

int main(void)
{
    test_basic_work_queue();
    test_work_queue_delay();
    test_work_queue_poll();
    test_container_of();
    test_additional_work_queue();
    test_work_queue_sync();
    return 0;
}