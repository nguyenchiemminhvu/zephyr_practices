#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    struct k_mem_slab pages;
    if (k_mem_slab_init(&pages, k_malloc(1024), 128, 8) != 0)
    {
        printk("Failed to initialize memory slab\n");
        return -1;
    }

    void *ptrs[10];
    for (int i = 0; i < 10; i++)
    {
        if (k_mem_slab_alloc(&pages, &ptrs[i], K_MSEC(100)) != 0)
        {
            ptrs[i] = NULL;
            printk("Failed to allocate memory from slab for index %d\n", i);
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (ptrs[i] != NULL)
        {
            // copy 127 chars to the allocated memory and null-terminate it
            memset(ptrs[i], 'A' + i, 127);
            ((char *)ptrs[i])[127] = '\0';
            printk("Allocated memory from slab for index %d: %s\n", i, (char *)ptrs[i]);
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (ptrs[i] != NULL)
        {
            k_mem_slab_free(&pages, &ptrs[i]);
        }
    }

    return 0;
}