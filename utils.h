#pragma once

#include "Arduino.h"

template <typename T>
void fmt_real_to_str(char *buf, T val, uint32_t precision = 3)
{
    int multiplier = 1;
    char fmt_string[8];
    // sprintf(fmt_string, "%d.%03d")
    for(int i = 0; i < precision; i++){ multiplier *= 10; }
    T frac = val - (int)val;
    frac = frac < 0 ? -frac : frac;
    int32_t fmt = frac * multiplier;
    sprintf(buf, "%d.%03d", (int)val, fmt);
};

template <typename T>
inline int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

template <typename T>
inline T random(const T &min, const T &max)
{
    return min + (max - min) * (rand() / (float) RAND_MAX);
}

template <typename T>
inline const T& clamp(const T &val, const T &min, const T &max)
{
    return val < min ? min : (val > max ? max : val);
}

template <typename T>
inline T mix(const T &lhs, const T &rhs, float ratio)
{
    return lhs + ratio * (rhs - lhs);
}

template <typename T>
inline T map_value(const T &val, const T &src_min, const T &src_max,
                   const T &dst_min, const T &dst_max)
{
    float mix_val = clamp<float>(val / (src_max - src_min), 0.f, 1.f);
    return mix<T>(dst_min, dst_max, mix_val);
}

/*! smoothstep performs smooth Hermite interpolation between 0 and 1,
 *  when edge0 < x < edge1.
 *  This is useful in cases where a threshold function with a smooth transition is desired
 */
inline float smoothstep(float edge0, float edge1, float x)
{
    float t = clamp<float>((x - edge0) / (edge1 - edge0), 0.f, 1.f);
    return t * t * (3.0 - 2.0 * t);
}
