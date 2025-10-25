#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <array>

namespace CE::Math
    {
        // Forward declarations
    class Quaternion;
    class Matrix4;

    class Vector2
        {
        public:
            float x, y;

            // Constructors
            Vector2 () : x ( 0.0f ), y ( 0.0f ) { }
            Vector2 ( float x, float y ) : x ( x ), y ( y ) { }
            explicit Vector2 ( float scalar ) : x ( scalar ), y ( scalar ) { }

            // Basic operations
            Vector2 operator+( const Vector2 & other ) const { return Vector2 ( x + other.x, y + other.y ); }
            Vector2 operator-( const Vector2 & other ) const { return Vector2 ( x - other.x, y - other.y ); }
            Vector2 operator*( const Vector2 & other ) const { return Vector2 ( x * other.x, y * other.y ); }
            Vector2 operator/( const Vector2 & other ) const { return Vector2 ( x / other.x, y / other.y ); }

            Vector2 operator*( float scalar ) const { return Vector2 ( x * scalar, y * scalar ); }
            Vector2 operator/( float scalar ) const { return Vector2 ( x / scalar, y / scalar ); }

            // Compound assignment
            Vector2 & operator+=( const Vector2 & other ) { x += other.x; y += other.y; return *this; }
            Vector2 & operator-=( const Vector2 & other ) { x -= other.x; y -= other.y; return *this; }
            Vector2 & operator*=( const Vector2 & other ) { x *= other.x; y *= other.y; return *this; }
            Vector2 & operator/=( const Vector2 & other ) { x /= other.x; y /= other.y; return *this; }

            Vector2 & operator*=( float scalar ) { x *= scalar; y *= scalar; return *this; }
            Vector2 & operator/=( float scalar ) { x /= scalar; y /= scalar; return *this; }

            // Comparison
            bool operator==( const Vector2 & other ) const {
                return std::abs ( x - other.x ) < std::numeric_limits<float>::epsilon () &&
                    std::abs ( y - other.y ) < std::numeric_limits<float>::epsilon ();
                }
            bool operator!=( const Vector2 & other ) const { return !( *this == other ); }

            // Utility functions
            float Length () const { return std::sqrt ( x * x + y * y ); }
            float LengthSquared () const { return x * x + y * y; }
            Vector2 Normalized () const {
                float len = Length ();
                return len > 0 ? Vector2 ( x / len, y / len ) : Vector2 ( 0.0f, 0.0f );
                }
            void Normalize () {
                float len = Length ();
                if (len > 0) { x /= len; y /= len; }
                }

            float Dot ( const Vector2 & other ) const { return x * other.x + y * other.y; }

            static float Distance ( const Vector2 & a, const Vector2 & b ) { return ( a - b ).Length (); }
            static float DistanceSquared ( const Vector2 & a, const Vector2 & b ) { return ( a - b ).LengthSquared (); }

            // Static constants
            static const Vector2 Zero;
            static const Vector2 One;
            static const Vector2 UnitX;
            static const Vector2 UnitY;
        };

    class Vector3
        {
        public:
            float x, y, z;

            // Constructors
            Vector3 () : x ( 0.0f ), y ( 0.0f ), z ( 0.0f ) { }
            Vector3 ( float x, float y, float z ) : x ( x ), y ( y ), z ( z ) { }
            explicit Vector3 ( float scalar ) : x ( scalar ), y ( scalar ), z ( scalar ) { }
            Vector3 ( const Vector2 & vec2, float z = 0.0f ) : x ( vec2.x ), y ( vec2.y ), z ( z ) { }

            // Basic operations (similar to Vector2 but extended to 3D)
            Vector3 operator+( const Vector3 & other ) const { return Vector3 ( x + other.x, y + other.y, z + other.z ); }
            Vector3 operator-( const Vector3 & other ) const { return Vector3 ( x - other.x, y - other.y, z - other.z ); }
            Vector3 operator*( const Vector3 & other ) const { return Vector3 ( x * other.x, y * other.y, z * other.z ); }
            Vector3 operator/( const Vector3 & other ) const { return Vector3 ( x / other.x, y / other.y, z / other.z ); }

            Vector3 operator*( float scalar ) const { return Vector3 ( x * scalar, y * scalar, z * scalar ); }
            Vector3 operator/( float scalar ) const { return Vector3 ( x / scalar, y / scalar, z / scalar ); }

            // Compound assignment
            Vector3 & operator+=( const Vector3 & other ) { x += other.x; y += other.y; z += other.z; return *this; }
            Vector3 & operator-=( const Vector3 & other ) { x -= other.x; y -= other.y; z -= other.z; return *this; }
            Vector3 & operator*=( const Vector3 & other ) { x *= other.x; y *= other.y; z *= other.z; return *this; }
            Vector3 & operator/=( const Vector3 & other ) { x /= other.x; y /= other.y; z /= other.z; return *this; }

            Vector3 & operator*=( float scalar ) { x *= scalar; y *= scalar; z *= scalar; return *this; }
            Vector3 & operator/=( float scalar ) { x /= scalar; y /= scalar; z /= scalar; return *this; }

            // Comparison
            bool operator==( const Vector3 & other ) const {
                return std::abs ( x - other.x ) < std::numeric_limits<float>::epsilon () &&
                    std::abs ( y - other.y ) < std::numeric_limits<float>::epsilon () &&
                    std::abs ( z - other.z ) < std::numeric_limits<float>::epsilon ();
                }
            bool operator!=( const Vector3 & other ) const { return !( *this == other ); }

            // Utility functions
            float Length () const { return std::sqrt ( x * x + y * y + z * z ); }
            float LengthSquared () const { return x * x + y * y + z * z; }
            Vector3 Normalized () const {
                float len = Length ();
                return len > 0 ? Vector3 ( x / len, y / len, z / len ) : Vector3 ( 0.0f, 0.0f, 0.0f );
                }
            void Normalize () {
                float len = Length ();
                if (len > 0) { x /= len; y /= len; z /= len; }
                }

            float Dot ( const Vector3 & other ) const { return x * other.x + y * other.y + z * other.z; }
            Vector3 Cross ( const Vector3 & other ) const {
                return Vector3 (
                    y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x
                );
                }

            static float Distance ( const Vector3 & a, const Vector3 & b ) { return ( a - b ).Length (); }
            static float DistanceSquared ( const Vector3 & a, const Vector3 & b ) { return ( a - b ).LengthSquared (); }

            // Static constants
            static const Vector3 Zero;
            static const Vector3 One;
            static const Vector3 UnitX;
            static const Vector3 UnitY;
            static const Vector3 UnitZ;
        };

    class Vector4
        {
        public:
            float x, y, z, w;

            // Конструкторы
            Vector4 () : x ( 0.0f ), y ( 0.0f ), z ( 0.0f ), w ( 0.0f ) { }
            Vector4 ( float x, float y, float z, float w ) : x ( x ), y ( y ), z ( z ), w ( w ) { }
            explicit Vector4 ( float scalar ) : x ( scalar ), y ( scalar ), z ( scalar ), w ( scalar ) { }
            Vector4 ( const Vector3 & vec3, float w = 1.0f ) : x ( vec3.x ), y ( vec3.y ), z ( vec3.z ), w ( w ) { }

            // Basic operations
            Vector4 operator+( const Vector4 & other ) const { return Vector4 ( x + other.x, y + other.y, z + other.z, w + other.w ); }
            Vector4 operator-( const Vector4 & other ) const { return Vector4 ( x - other.x, y - other.y, z - other.z, w - other.w ); }
            Vector4 operator*( const Vector4 & other ) const { return Vector4 ( x * other.x, y * other.y, z * other.z, w * other.w ); }
            Vector4 operator/( const Vector4 & other ) const { return Vector4 ( x / other.x, y / other.y, z / other.z, w / other.w ); }

            Vector4 operator*( float scalar ) const { return Vector4 ( x * scalar, y * scalar, z * scalar, w * scalar ); }
            Vector4 operator/( float scalar ) const { return Vector4 ( x / scalar, y / scalar, z / scalar, w / scalar ); }

            // Compound assignment
            Vector4 & operator+=( const Vector4 & other ) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
            Vector4 & operator-=( const Vector4 & other ) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
            Vector4 & operator*=( const Vector4 & other ) { x *= other.x; y *= other.y; z *= other.z; w *= other.w; return *this; }
            Vector4 & operator/=( const Vector4 & other ) { x /= other.x; y /= other.y; z /= other.z; w /= other.w; return *this; }

            Vector4 & operator*=( float scalar ) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
            Vector4 & operator/=( float scalar ) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

            // Comparison
            bool operator==( const Vector4 & other ) const {
                return std::abs ( x - other.x ) < std::numeric_limits<float>::epsilon () &&
                    std::abs ( y - other.y ) < std::numeric_limits<float>::epsilon () &&
                    std::abs ( z - other.z ) < std::numeric_limits<float>::epsilon () &&
                    std::abs ( w - other.w ) < std::numeric_limits<float>::epsilon ();
                }
            bool operator!=( const Vector4 & other ) const { return !( *this == other ); }

            // Utility functions
            float Length () const { return std::sqrt ( x * x + y * y + z * z + w * w ); }
            float LengthSquared () const { return x * x + y * y + z * z + w * w; }
            Vector4 Normalized () const {
                float len = Length ();
                return len > 0 ? Vector4 ( x / len, y / len, z / len, w / len ) : Vector4 ( 0.0f, 0.0f, 0.0f, 0.0f );
                }
            void Normalize () {
                float len = Length ();
                if (len > 0) { x /= len; y /= len; z /= len; w /= len; }
                }

            float Dot ( const Vector4 & other ) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

            static float Distance ( const Vector4 & a, const Vector4 & b ) { return ( a - b ).Length (); }
            static float DistanceSquared ( const Vector4 & a, const Vector4 & b ) { return ( a - b ).LengthSquared (); }

            // Static constants
            static const Vector4 Zero;
            static const Vector4 One;
            static const Vector4 UnitX;
            static const Vector4 UnitY;
            static const Vector4 UnitZ;
            static const Vector4 UnitW;
        };

    } // namespace ChudEngine::Math