#ifndef SIMPLE_MATH_H
#define SIMPLE_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Add two 32-bit signed integers.
 * @param a First operand.
 * @param b Second operand.
 * @return Sum of a and b.
 */
int32_t math_add(int32_t a, int32_t b);

/**
 * @brief Multiply two 32-bit signed integers.
 * @param a First operand.
 * @param b Second operand.
 * @return Product of a and b.
 */
int32_t math_multiply(int32_t a, int32_t b);

/**
 * @brief Compute integer power.
 * @param base  Base value.
 * @param exp   Non-negative exponent.
 * @return base raised to the power exp (returns 1 for exp == 0).
 */
int32_t math_power(int32_t base, uint8_t exp);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLE_MATH_H */