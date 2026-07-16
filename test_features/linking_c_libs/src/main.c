#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "simple_math.h"
#include "unit_converter.h"

int main(void)
{
    printk("=== Zephyr Static Library Linking Demo ===\n\n");

    /* ----- simple_math ----- */
    int32_t sum  = math_add(25, 17);
    int32_t prod = math_multiply(6, 7);
    int32_t pwr  = math_power(2, 10);

    printk("[simple_math]\n");
    printk("  25 + 17   = %d\n", sum);
    printk("  6  * 7    = %d\n", prod);
    printk("  2 ^ 10    = %d\n\n", pwr);

    /* ----- unit_converter (fixed-point x100 / x1000) ----- */
    int32_t temp_c  = 2500;   /* 25.00 °C */
    int32_t temp_f  = converter_celsius_to_fahrenheit(temp_c);

    int32_t dist_km = 10000;  /* 10.000 km */
    int32_t dist_mi = converter_km_to_miles(dist_km);

    int32_t wt_kg   = 75000;  /* 75.000 kg */
    int32_t wt_lb   = converter_kg_to_pounds(wt_kg);

    printk("[unit_converter]  (fixed-point)\n");
    printk("  %d.%02d C  -> %d.%02d F\n",
           temp_c / 100, temp_c % 100,
           temp_f / 100, temp_f % 100);
    printk("  %d.%03d km -> %d.%03d mi\n",
           dist_km / 1000, dist_km % 1000,
           dist_mi / 1000, dist_mi % 1000);
    printk("  %d.%03d kg -> %d.%03d lb\n",
           wt_kg / 1000, wt_kg % 1000,
           wt_lb / 1000, wt_lb % 1000);

    printk("\nDone.\n");
    return 0;
}