#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct sample_data_type
{
    int a;
    int b;
};

#define DEFINE_SAMPLE_DATA(name, val_a, val_b) \
    STRUCT_SECTION_ITERABLE(sample_data_type, name) = { .a = val_a, .b = val_b }

DEFINE_SAMPLE_DATA(sample_data_1, 1, 2);
DEFINE_SAMPLE_DATA(sample_data_2, 3, 4);

int main(void)
{
    printk("Starting kernel iterable example...\n");
    printk("Iterating over sample data:\n");
    STRUCT_SECTION_FOREACH(sample_data_type, data)
    {
        printk("Data: a = %d, b = %d\n", data->a, data->b);
    }
    return 0;
}