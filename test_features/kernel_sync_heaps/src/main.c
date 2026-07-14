#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

K_HEAP_DEFINE(global_heap, 1024);

void safe_free_wrapper(struct k_heap *heap, void* ptr)
{
    if (ptr == NULL || heap == NULL)
    {
        return;
    }

    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t heap_start = (uintptr_t)heap->heap.init_mem;
    uintptr_t heap_end = heap_start + heap->heap.init_bytes;
    if (addr < heap_start || addr >= heap_end)
    {
        printk("Pointer %p is not within the specified heap range\n", ptr);
        return;
    }
    k_heap_free(heap, ptr);
}

int main(void)
{
    struct k_heap local_heap;
    k_heap_init(&local_heap, k_malloc(512), 512);

    void *ptr = k_heap_alloc(&local_heap, 1024, K_MSEC(1000));
    if (ptr == NULL)
    {
        printk("Failed to allocate memory from local heap\n");
    }
    else
    {
        printk("Allocated memory from local heap: %p\n", ptr);
        safe_free_wrapper(&local_heap, ptr);
    }

    ptr = k_heap_alloc(&global_heap, 512, K_MSEC(1000));
    if (ptr == NULL)
    {
        printk("Failed to allocate memory from global heap\n");
    }
    else
    {
        printk("Allocated memory from global heap: %p\n", ptr);
        safe_free_wrapper(&global_heap, ptr);
    }

    ptr = k_heap_alloc(&global_heap, 255, K_MSEC(1000));
    if (ptr == NULL)
    {
        printk("Failed to allocate memory from global heap\n");
    }
    else
    {
        printk("Allocated memory from global heap: %p\n", ptr);
        safe_free_wrapper(&global_heap, ptr);
    }

    return 0;
}