#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

RING_BUF_DECLARE(global_ringbuf, 255);

int main(void)
{
    uint8_t local_buffer_mem[255];
    struct ring_buf local_ringbuf;
    ring_buf_init(&local_ringbuf, sizeof(local_buffer_mem), local_buffer_mem);

    uint8_t test_data[] = {1, 2, 3, 4, 5};

    for (int i = 0; i < sizeof(test_data); i++) {
        ring_buf_put(&global_ringbuf, &test_data[i], 1);
        ring_buf_put(&local_ringbuf, &test_data[i], 1);
    }

    printk("Global ring buffer contents:\n");
    uint8_t data;
    while (ring_buf_get(&global_ringbuf, &data, 1) > 0)
    {
        printk("%d ", (int)data);
    }

    printk("\nLocal ring buffer contents:\n");
    while (ring_buf_get(&local_ringbuf, &data, 1) > 0)
    {
        printk("%d ", (int)data);
    }

    return 0;
}