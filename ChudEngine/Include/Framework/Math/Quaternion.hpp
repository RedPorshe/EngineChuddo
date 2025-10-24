#pragma once

#include "Vector.hpp"
// Убрано #include "Matrix.hpp" - будет forward declaration
#include <cmath>

namespace CE::Math
    {
    class Matrix4; // Добавлено forward declaration

    class Quaternion
        {
        public:
            float x, y, z, w;

            // Constructors
            Quaternion ();
            Quaternion ( float x, float y, float z, float w );
            Quaternion ( const Vector3 & axis, float angle );
            Quaternion ( const Vector3 & eulerAngles ); // Pitch, Yaw, Roll

            // Basic operations
            Quaternion operator+( const Quaternion & other ) const;
            Quaternion operator-( const Quaternion & other ) const;
            Quaternion operator*( const Quaternion & other ) const;
            Quaternion operator*( float scalar ) const;
            Quaternion operator/( float scalar ) const;

            // Compound assignment
            Quaternion & operator+=( const Quaternion & other );
            Quaternion & operator-=( const Quaternion & other );
            Quaternion & operator*=( const Quaternion & other );
            Quaternion & operator*=( float scalar );
            Quaternion & operator/=( float scalar );

            // Comparison
            bool operator==( const Quaternion & other ) const;
            bool operator!=( const Quaternion & other ) const;

            // Utility functions
            float Length () const;
            float LengthSquared () const;
            Quaternion Normalized () const;
            void Normalize ();
            Quaternion Conjugate () const;
            Quaternion Inverse () const;

            // Rotation operations
            Vector3 Rotate ( const Vector3 & point ) const;
            Matrix4 ToMatrix () const;
            Vector3 ToEulerAngles () const;

            // Static factory methods
            static Quaternion Identity ();
            static Quaternion FromAxisAngle ( const Vector3 & axis, float angle );
            static Quaternion FromEulerAngles ( const Vector3 & eulerAngles );
            static Quaternion FromEulerAngles ( float pitch, float yaw, float roll );
            static Quaternion LookRotation ( const Vector3 & forward, const Vector3 & up = Vector3::UnitY );

            // Interpolation
            static Quaternion Lerp ( const Quaternion & a, const Quaternion & b, float t );
            static Quaternion Slerp ( const Quaternion & a, const Quaternion & b, float t );

            // Static constants
            static const Quaternion IdentityQuaternion;
        };

    } // namespace ChudEngine::Math