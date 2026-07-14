#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static uint8_t __aligned(1) nmea_buffer[1024]; // Buffer for the pipe

struct nmea_message
{
    const char *message;
    size_t length;
};

void producer_thread_func(void *arg1, void *arg2, void *arg3)
{
    struct k_pipe *nmea_pipe = (struct k_pipe *)arg1;

    int rc = 0;
    const char* sample_nmea = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";

    while (1)
    {
        struct nmea_message msg;
        msg.message = sample_nmea;
        msg.length = strlen(sample_nmea);

        // Write the NMEA message to the pipe
        rc = k_pipe_write(nmea_pipe, msg.message, msg.length, K_NO_WAIT);
        if (rc <= 0)
        {
            printk("Producer: Failed to write to pipe, rc=%d\n", rc);
        }
        else
        {
            printk("Producer: Wrote %d bytes to pipe\n", rc);
        }

        k_sleep(K_MSEC(1000)); // Sleep for 1 second before sending the next message
    }
}

void consumer_thread_func(void *arg1, void *arg2, void *arg3)
{
    struct k_pipe *nmea_pipe = (struct k_pipe *)arg1;

    int rc = 0;
    while (1)
    {
        struct nmea_message msg;
        msg.message = (char*)k_malloc(1024); // Allocate memory for the message

        // Read the NMEA message from the pipe
        rc = k_pipe_read(nmea_pipe, msg.message, 65, K_FOREVER);
        if (rc <= 0)
        {
            printk("Consumer: Failed to read from pipe, rc=%d\n", rc);
        }
        else
        {
            msg.length = rc;
            printk("Consumer: Read %d bytes from pipe: %.*s\n", msg.length, (int)msg.length, msg.message);
            if (strcmp(msg.message, (char*)nmea_buffer) == 0)
            {
                printk("Consumer: Received the expected NMEA message.\n");
            }
            else
            {
                printk("Consumer: Received an unexpected NMEA message.\n");
            }
        }

        k_free(msg.message); // Free the allocated memory
    }
}

#define THREAD_STACK_SIZE 1024
K_THREAD_STACK_DEFINE(producer_stack, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(consumer_stack, THREAD_STACK_SIZE);

struct k_thread producer_thread_data;
struct k_thread consumer_thread_data;

int main(void)
{
    struct k_pipe nmea_pipe;
    memset(nmea_buffer, 0, sizeof(nmea_buffer)); // Initialize the buffer to zero
    k_pipe_init(&nmea_pipe, nmea_buffer, sizeof(nmea_buffer));

    k_tid_t producer_tid = k_thread_create(&producer_thread_data, producer_stack, THREAD_STACK_SIZE,
                                            producer_thread_func, &nmea_pipe, NULL, NULL,
                                            1, 0, K_NO_WAIT);
    k_tid_t consumer_tid = k_thread_create(&consumer_thread_data, consumer_stack, THREAD_STACK_SIZE,
                                            consumer_thread_func, &nmea_pipe, NULL, NULL,
                                            1, 0, K_NO_WAIT);

    k_sleep(K_FOREVER); // Keep the main thread alive to allow producer and consumer threads to run
    return 0;
}