#pragma once

#include <cmath>
#include <limits>
#include <algorithm>

namespace CE::Math
    {

// Mathematical constants
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float HALF_PI = PI / 2.0f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float Epsilon = std::numeric_limits<float>::epsilon ();

    // Utility functions
    template<typename T>
    constexpr T Clamp ( T value, T min, T max ) {
        return ( value < min ) ? min : ( value > max ) ? max : value;
        }

    template<typename T>
    constexpr T Lerp ( T a, T b, float t ) {
        return a + ( b - a ) * Clamp ( t, 0.0f, 1.0f );
        }

    template<typename T>
    constexpr T Min ( T a, T b ) {
        return ( a < b ) ? a : b;
        }

    template<typename T>
    constexpr T Max ( T a, T b ) {
        return ( a > b ) ? a : b;
        }

    template<typename T>
    constexpr T Abs ( T value ) {
        return ( value < 0 ) ? -value : value;
        }

    inline bool Approximately ( float a, float b, float tolerance = Epsilon ) {
        return Abs ( a - b ) <= tolerance;
        }

    inline float ToRadians ( float degrees ) {
        return degrees * DEG_TO_RAD;
        }

    inline float ToDegrees ( float radians ) {
        return radians * RAD_TO_DEG;
        }

    inline float SmoothStep ( float edge0, float edge1, float x ) {
        x = Clamp ( ( x - edge0 ) / ( edge1 - edge0 ), 0.0f, 1.0f );
        return x * x * ( 3.0f - 2.0f * x );
        }

    inline float Sign ( float value ) {
        return ( value > 0.0f ) ? 1.0f : ( value < 0.0f ) ? -1.0f : 0.0f;
        }

    inline bool IsPowerOfTwo ( int value ) {
        return ( value > 0 ) && ( ( value & ( value - 1 ) ) == 0 );
        }

    inline int NextPowerOfTwo ( int value ) {
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value++;
        return value;
        }

    } // namespace ChudEngine::Math