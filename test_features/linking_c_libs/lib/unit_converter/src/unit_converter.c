#include <stdint.h>
#include "unit_converter.h"

/* Fixed-point arithmetic avoids floating-point, suitable for MCUs without FPU */

int32_t converter_celsius_to_fahrenheit(int32_t celsius)
{
    /* F = C * 9/5 + 32  (all values scaled x100) */
    return (celsius * 9 / 5) + 3200;
}

int32_t converter_km_to_miles(int32_t km)
{
    /* 1 km = 0.621371 miles; scaled x1000 → factor 621 */
    return (int32_t)((int64_t)km * 621 / 1000);
}

int32_t converter_kg_to_pounds(int32_t kg)
{
    /* 1 kg = 2.20462 lbs; scaled x1000 → factor 2205 */
    return (int32_t)((int64_t)kg * 2205 / 1000);
}