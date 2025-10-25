// MathFunctions.hpp
#pragma once
#include "Vector.hpp"

namespace CE::Math
    {
    inline Vector3 Normalize ( const Vector3 & vec ) {
        float len = vec.Length ();
        return ( len > 0.0f ) ? vec / len : Vector3::Zero;
        }

    inline float Dot ( const Vector3 & a, const Vector3 & b ) {
        return a.Dot ( b );
        }

    inline Vector3 Cross ( const Vector3 & a, const Vector3 & b ) {
        return a.Cross ( b );
        }
    }