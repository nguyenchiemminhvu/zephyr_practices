#ifndef UNIT_CONVERTER_H
#define UNIT_CONVERTER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert temperature from Celsius to Fahrenheit.
 * @param celsius Temperature in degrees Celsius (scaled x100 for fixed-point).
 * @return Temperature in degrees Fahrenheit (scaled x100).
 */
int32_t converter_celsius_to_fahrenheit(int32_t celsius);

/**
 * @brief Convert distance from kilometers to miles.
 * @param km Distance in kilometers (scaled x1000 for fixed-point).
 * @return Distance in miles (scaled x1000).
 */
int32_t converter_km_to_miles(int32_t km);

/**
 * @brief Convert weight from kilograms to pounds.
 * @param kg Weight in kilograms (scaled x1000 for fixed-point).
 * @return Weight in pounds (scaled x1000).
 */
int32_t converter_kg_to_pounds(int32_t kg);

#ifdef __cplusplus
}
#endif

#endif /* UNIT_CONVERTER_H */