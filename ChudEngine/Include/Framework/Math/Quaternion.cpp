#include "Math/Quaternion.hpp"
#include "Math/Matrix.hpp"
#include <cmath>
#include <limits>

namespace CE::Math
    {
    Quaternion::Quaternion () : x ( 0.0f ), y ( 0.0f ), z ( 0.0f ), w ( 1.0f ) { }

    Quaternion::Quaternion ( float x, float y, float z, float w )
        : x ( x ), y ( y ), z ( z ), w ( w ) { }

    Quaternion::Quaternion ( const Vector3 & axis, float angle ) {
        float halfAngle = angle * 0.5f;
        float sinHalf = std::sin ( halfAngle );
        float cosHalf = std::cos ( halfAngle );

        Vector3 normalizedAxis = axis.Normalized ();
        x = normalizedAxis.x * sinHalf;
        y = normalizedAxis.y * sinHalf;
        z = normalizedAxis.z * sinHalf;
        w = cosHalf;
        }

    Quaternion::Quaternion ( const Vector3 & eulerAngles ) {
        // Убедитесь, что углы в радианах
        float halfPitch = eulerAngles.x * 0.5f;
        float halfYaw = eulerAngles.y * 0.5f;
        float halfRoll = eulerAngles.z * 0.5f;

        float sinPitch = std::sin ( halfPitch );
        float cosPitch = std::cos ( halfPitch );
        float sinYaw = std::sin ( halfYaw );
        float cosYaw = std::cos ( halfYaw );
        float sinRoll = std::sin ( halfRoll );
        float cosRoll = std::cos ( halfRoll );

        // Правильная формула для порядка Yaw-Pitch-Roll
        x = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
        y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
        z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
        w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
        }

    Quaternion Quaternion::operator+( const Quaternion & other ) const {
        return Quaternion ( x + other.x, y + other.y, z + other.z, w + other.w );
        }

    Quaternion Quaternion::operator-( const Quaternion & other ) const {
        return Quaternion ( x - other.x, y - other.y, z - other.z, w - other.w );
        }

    Quaternion Quaternion::operator*( const Quaternion & other ) const {
        return Quaternion (
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        );
        }

    Quaternion Quaternion::operator*( float scalar ) const {
        return Quaternion ( x * scalar, y * scalar, z * scalar, w * scalar );
        }

    Quaternion Quaternion::operator/( float scalar ) const {
        return Quaternion ( x / scalar, y / scalar, z / scalar, w / scalar );
        }

    Quaternion & Quaternion::operator+=( const Quaternion & other ) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
        }

    Quaternion & Quaternion::operator-=( const Quaternion & other ) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
        }

    Quaternion & Quaternion::operator*=( const Quaternion & other ) {
        *this = *this * other;
        return *this;
        }

    Quaternion & Quaternion::operator*=( float scalar ) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
        }

    Quaternion & Quaternion::operator/=( float scalar ) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
        }

    bool Quaternion::operator==( const Quaternion & other ) const {
        return std::abs ( x - other.x ) < std::numeric_limits<float>::epsilon () &&
            std::abs ( y - other.y ) < std::numeric_limits<float>::epsilon () &&
            std::abs ( z - other.z ) < std::numeric_limits<float>::epsilon () &&
            std::abs ( w - other.w ) < std::numeric_limits<float>::epsilon ();
        }

    bool Quaternion::operator!=( const Quaternion & other ) const {
        return !( *this == other );
        }

    float Quaternion::Length () const {
        return std::sqrt ( x * x + y * y + z * z + w * w );
        }

    float Quaternion::LengthSquared () const {
        return x * x + y * y + z * z + w * w;
        }

    Quaternion Quaternion::Normalized () const {
        float len = Length ();
        return len > 0 ? Quaternion ( x / len, y / len, z / len, w / len ) : Quaternion ();
        }

    void Quaternion::Normalize () {
        float len = Length ();
        if (len > 0)
            {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
            }
        }

    Quaternion Quaternion::Conjugate () const {
        return Quaternion ( -x, -y, -z, w );
        }

    Quaternion Quaternion::Inverse () const {
        float lenSq = LengthSquared ();
        if (lenSq > 0)
            {
            return Conjugate () * ( 1.0f / lenSq );
            }
        return Quaternion ();
        }

    Vector3 Quaternion::Rotate ( const Vector3 & point ) const {
        // Convert point to quaternion
        Quaternion pointQuat ( point.x, point.y, point.z, 0.0f );
        Quaternion result = ( *this ) * pointQuat * Conjugate ();
        return Vector3 ( result.x, result.y, result.z );
        }

    Matrix4 Quaternion::ToMatrix () const {
        Matrix4 result;

        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float xy = x * y;
        float xz = x * z;
        float xw = x * w;
        float yz = y * z;
        float yw = y * w;
        float zw = z * w;

        result.elements[ 0 ] = 1.0f - 2.0f * ( yy + zz );
        result.elements[ 1 ] = 2.0f * ( xy + zw );
        result.elements[ 2 ] = 2.0f * ( xz - yw );

        result.elements[ 4 ] = 2.0f * ( xy - zw );
        result.elements[ 5 ] = 1.0f - 2.0f * ( xx + zz );
        result.elements[ 6 ] = 2.0f * ( yz + xw );

        result.elements[ 8 ] = 2.0f * ( xz + yw );
        result.elements[ 9 ] = 2.0f * ( yz - xw );
        result.elements[ 10 ] = 1.0f - 2.0f * ( xx + yy );

        result.elements[ 15 ] = 1.0f;

        return result;
        }

    Vector3 Quaternion::ToEulerAngles () const {
        Vector3 euler;

        // Roll (x-axis rotation)
        float sinRoll = 2.0f * ( w * x + y * z );
        float cosRoll = 1.0f - 2.0f * ( x * x + y * y );
        euler.z = std::atan2 ( sinRoll, cosRoll );

        // Pitch (y-axis rotation)
        float sinPitch = 2.0f * ( w * y - z * x );
        if (std::abs ( sinPitch ) >= 1.0f)
            {
            euler.x = std::copysign ( 3.14159265358979323846f / 2.0f, sinPitch );
            }
        else
            {
            euler.x = std::asin ( sinPitch );
            }

            // Yaw (z-axis rotation)
        float sinYaw = 2.0f * ( w * z + x * y );
        float cosYaw = 1.0f - 2.0f * ( y * y + z * z );
        euler.y = std::atan2 ( sinYaw, cosYaw );

        return euler;
        }

    Quaternion Quaternion::Identity () {
        return Quaternion ( 0.0f, 0.0f, 0.0f, 1.0f );
        }

    Quaternion Quaternion::FromAxisAngle ( const Vector3 & axis, float angle ) {
        return Quaternion ( axis, angle );
        }

    Quaternion Quaternion::FromEulerAngles ( const Vector3 & eulerAngles ) {
        return Quaternion ( eulerAngles );
        }

    Quaternion Quaternion::FromEulerAngles ( float pitch, float yaw, float roll ) {
        return Quaternion ( Vector3 ( pitch, yaw, roll ) );
        }

    Quaternion Quaternion::LookRotation ( const Vector3 & forward, const Vector3 & up ) {
        Vector3 z = forward.Normalized () * -1.f;
        Vector3 x = up.Cross ( z ).Normalized ();
        Vector3 y = z.Cross ( x );

        float trace = x.x + y.y + z.z;
        Quaternion result;

        if (trace > 0.0f)
            {
            float s = 0.5f / std::sqrt ( trace + 1.0f );
            result.w = 0.25f / s;
            result.x = ( y.z - z.y ) * s;
            result.y = ( z.x - x.z ) * s;
            result.z = ( x.y - y.x ) * s;
            }
        else
            {
            if (x.x > y.y && x.x > z.z)
                {
                float s = 2.0f * std::sqrt ( 1.0f + x.x - y.y - z.z );
                result.w = ( y.z - z.y ) / s;
                result.x = 0.25f * s;
                result.y = ( y.x + x.y ) / s;
                result.z = ( z.x + x.z ) / s;
                }
            else if (y.y > z.z)
                {
                float s = 2.0f * std::sqrt ( 1.0f + y.y - x.x - z.z );
                result.w = ( z.x - x.z ) / s;
                result.x = ( y.x + x.y ) / s;
                result.y = 0.25f * s;
                result.z = ( z.y + y.z ) / s;
                }
            else
                {
                float s = 2.0f * std::sqrt ( 1.0f + z.z - x.x - y.y );
                result.w = ( x.y - y.x ) / s;
                result.x = ( z.x + x.z ) / s;
                result.y = ( z.y + y.z ) / s;
                result.z = 0.25f * s;
                }
            }

        return result.Normalized ();
        }

    Quaternion Quaternion::Lerp ( const Quaternion & a, const Quaternion & b, float t ) {
        float tClamped = std::clamp ( t, 0.0f, 1.0f );
        return Quaternion (
            a.x + ( b.x - a.x ) * tClamped,
            a.y + ( b.y - a.y ) * tClamped,
            a.z + ( b.z - a.z ) * tClamped,
            a.w + ( b.w - a.w ) * tClamped
        ).Normalized ();
        }

    Quaternion Quaternion::Slerp ( const Quaternion & a, const Quaternion & b, float t ) {
        float tClamped = std::clamp ( t, 0.0f, 1.0f );

        float cosHalfTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

        if (std::abs ( cosHalfTheta ) >= 1.0f)
            {
            return a;
            }

        float halfTheta = std::acos ( cosHalfTheta );
        float sinHalfTheta = std::sqrt ( 1.0f - cosHalfTheta * cosHalfTheta );

        if (std::abs ( sinHalfTheta ) < 0.001f)
            {
            return Quaternion (
                ( a.x * 0.5f + b.x * 0.5f ),
                ( a.y * 0.5f + b.y * 0.5f ),
                ( a.z * 0.5f + b.z * 0.5f ),
                ( a.w * 0.5f + b.w * 0.5f )
            );
            }

        float ratioA = std::sin ( ( 1 - tClamped ) * halfTheta ) / sinHalfTheta;
        float ratioB = std::sin ( tClamped * halfTheta ) / sinHalfTheta;

        return Quaternion (
            a.x * ratioA + b.x * ratioB,
            a.y * ratioA + b.y * ratioB,
            a.z * ratioA + b.z * ratioB,
            a.w * ratioA + b.w * ratioB
        );
        }

        // Static constants
    const Quaternion Quaternion::IdentityQuaternion = Quaternion ( 0.0f, 0.0f, 0.0f, 1.0f );

    } // namespace ChudEngine::Math