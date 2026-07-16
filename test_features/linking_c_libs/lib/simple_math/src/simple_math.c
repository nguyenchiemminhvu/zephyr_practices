#include <stdint.h>
#include "simple_math.h"

int32_t math_add(int32_t a, int32_t b)
{
    return a + b;
}

int32_t math_multiply(int32_t a, int32_t b)
{
    return a * b;
}

int32_t math_power(int32_t base, uint8_t exp)
{
    int32_t result = 1;

    for (uint8_t i = 0; i < exp; i++) {
        result *= base;
    }

    return result;
}