#pragma once

#include <array>
#include "Vector.hpp"

namespace CE::Math
    {
    class Quaternion; // Forward declaration

    class Matrix4
        {
        public:
            // Column-major storage (compatible with OpenGL/Vulkan)
            std::array<float, 16> elements;

            // Constructors
            Matrix4 ();
            explicit Matrix4 ( float diagonal );
            Matrix4 ( const std::array<float, 16> & elements );

            // Accessors
            float & operator[]( size_t index ) { return elements[ index ]; }
            const float & operator[]( size_t index ) const { return elements[ index ]; }

            float & At ( size_t row, size_t column ) { return elements[ column * 4 + row ]; }
            const float & At ( size_t row, size_t column ) const { return elements[ column * 4 + row ]; }

            static Matrix4 Rotate ( const Quaternion & rotation );
            Matrix4 Inverted () const;
            bool Invert ();

            // Basic operations
            Matrix4 operator+( const Matrix4 & other ) const;
            Matrix4 operator-( const Matrix4 & other ) const;
            Matrix4 operator*( const Matrix4 & other ) const;
            Matrix4 operator*( float scalar ) const;
            Vector4 operator*( const Vector4 & vector ) const;

            // Compound assignment
            Matrix4 & operator+=( const Matrix4 & other );
            Matrix4 & operator-=( const Matrix4 & other );
            Matrix4 & operator*=( const Matrix4 & other );
            Matrix4 & operator*=( float scalar );

            // Comparison
            bool operator==( const Matrix4 & other ) const;
            bool operator!=( const Matrix4 & other ) const;

            // Utility functions
            Matrix4 Transposed () const;
            void Transpose ();

            float Determinant () const;

            // Static factory methods
            static Matrix4 Identity ();
            static Matrix4 Translation ( const Vector3 & translation );
            static Matrix4 RotationX ( float angle );
            static Matrix4 RotationY ( float angle );
            static Matrix4 RotationZ ( float angle );
            static Matrix4 Rotation ( const Vector3 & axis, float angle );
            static Matrix4 Scale ( const Vector3 & scale );

            // Projection matrices
            static Matrix4 Orthographic ( float left, float right, float bottom, float top, float near, float far );
            static Matrix4 Perspective ( float fov, float aspect, float near, float far );

            // View matrix
            static Matrix4 LookAt ( const Vector3 & eye, const Vector3 & target, const Vector3 & up );

              // Диагностические методы
            void DebugPrint ( const char * name = "Matrix4" ) const;
            bool IsValid () const;

            // Для проверки выравнивания
            static constexpr size_t Size () { return 16 * sizeof ( float ); }
            static constexpr size_t SizeBytes () { return 16 * sizeof ( float ); }

              // Конвертирует column-major в row-major (для Vulkan/GLSL)
            Matrix4 ToGLSLCompatible () const { return Transposed (); }

            // Создает матрицы уже в row-major формате
            static Matrix4 PerspectiveGLSL ( float fov, float aspect, float near, float far );
            static Matrix4 LookAtGLSL ( const Vector3 & eye, const Vector3 & target, const Vector3 & up );

            // Static constants
            static const Matrix4 Zero;
            static const Matrix4 IdentityMatrix;
        };

    } // namespace ChudEngine::Math