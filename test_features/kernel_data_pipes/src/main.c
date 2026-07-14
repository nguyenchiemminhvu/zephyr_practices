#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static uint8_t __aligned(1) nmea_buffer[1024]; // Buffer for the pipe

struct nmea_message
{
    char *message;
    size_t length;
};

void producer_thread_func(void *arg1, void *arg2, void *arg3)
{
    struct k_pipe *nmea_pipe = (struct k_pipe *)arg1;

    int rc = 0;
    const char* sample_nmea = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    size_t total_size = 1024; // Total size of the NMEA messages
    {

        // build pipe payload
        msg.message = k_malloc(1025); // Allocate memory for the message
        memset(msg.message, 0, 1025); // Initialize the message buffer to zero
        msg.length = 0; // Initialize the message length to zero
        while (msg.length < total_size)
        {
            size_t remaining = total_size - msg.length;
            size_t chunk_size = (remaining < strlen(sample_nmea)) ? remaining : strlen(sample_nmea);
            memcpy(msg.message + msg.length, sample_nmea, chunk_size);
            msg.length += chunk_size;
        }
        msg.message[msg.length] = '\0'; // Null-terminate the message

        // Write the NMEA message to the pipe
        size_t bytes_written = 0;
        while (bytes_written < msg.length)
        {
            rc = k_pipe_write(nmea_pipe, msg.message + bytes_written, msg.length - bytes_written, K_NO_WAIT);
            if (rc < 0)
            {
                printk("Producer: Failed to write to pipe, rc=%d\n", rc);
                break;
            }
            bytes_written += rc;
        }

        k_free(msg.message); // Free the allocated memory
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
        msg.message = (char*)k_malloc(1025); // Allocate memory for the message
        memset(msg.message, 0, 1025); // Initialize the message buffer to zero

        size_t bytes_read = 0;
        while (bytes_read < 1024)
        {
            rc = k_pipe_read(nmea_pipe, msg.message + bytes_read, 1024 - bytes_read, K_FOREVER);
            if (rc < 0)
            {
                printk("Consumer: Failed to read from pipe, rc=%d\n", rc);
                break;
            }
            bytes_read += rc;
        }
        msg.message[bytes_read] = '\0'; // Null-terminate the message

        // Check if the received data successfully starts with the NMEA prefix
        if (strncmp(msg.message, "$GPGGA", 6) == 0)
        {
            printk("Consumer: Received valid NMEA message sequence.\n");
        }
        else
        {
            printk("Consumer: Received unexpected message format!\n");
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