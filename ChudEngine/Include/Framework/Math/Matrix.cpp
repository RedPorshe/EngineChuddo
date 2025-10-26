#include "Math/Matrix.hpp"
#include "Math/Quaternion.hpp"
#include "Utils/Logger.hpp"
#include <cmath>
#include <array> 
#include <sstream>

namespace CE::Math
    {
    Matrix4::Matrix4 () : elements { 0 } { }

    Matrix4::Matrix4 ( float diagonal ) : elements { 0 } {
        elements[ 0 ] = diagonal;
        elements[ 5 ] = diagonal;
        elements[ 10 ] = diagonal;
        elements[ 15 ] = diagonal;
        }

    Matrix4::Matrix4 ( const std::array<float, 16> & elements ) : elements ( elements ) { }

    Matrix4 Matrix4::PerspectiveGLSL ( float fov, float aspect, float near, float far )
        {
        Matrix4 result;

        float tanHalfFov = std::tan ( fov / 2.0f );
        float range = near - far;

        // Row-major perspective matrix
        result.elements[ 0 ] = 1.0f / ( aspect * tanHalfFov );
        result.elements[ 5 ] = 1.0f / tanHalfFov;
        result.elements[ 10 ] = ( -near - far ) / range;
        result.elements[ 11 ] = 1.0f;
        result.elements[ 14 ] = ( 2.0f * far * near ) / range;
        result.elements[ 15 ] = 0.0f;

        // Остальные элементы = 0
        return result;
        }

    Matrix4 Matrix4::LookAtGLSL ( const Vector3 & eye, const Vector3 & target, const Vector3 & up )
        {
        Vector3 zAxis = ( target - eye ).Normalized ();
        Vector3 xAxis = up.Cross ( zAxis ).Normalized ();
        Vector3 yAxis = zAxis.Cross ( xAxis );

        Matrix4 result ( 1.0f );

        // Row-major lookat matrix
        result.elements[ 0 ] = xAxis.x;
        result.elements[ 1 ] = xAxis.y;
        result.elements[ 2 ] = xAxis.z;
        result.elements[ 3 ] = -xAxis.Dot ( eye );

        result.elements[ 4 ] = yAxis.x;
        result.elements[ 5 ] = yAxis.y;
        result.elements[ 6 ] = yAxis.z;
        result.elements[ 7 ] = -yAxis.Dot ( eye );

        result.elements[ 8 ] = zAxis.x;
        result.elements[ 9 ] = zAxis.y;
        result.elements[ 10 ] = zAxis.z;
        result.elements[ 11 ] = -zAxis.Dot ( eye );

        return result;
        }

    void Matrix4::DebugPrint ( const char * name ) const
        {
        std::stringstream ss;
        ss << name << ":\n";
        for (int row = 0; row < 4; ++row)
            {
            ss << "  ";
            for (int col = 0; col < 4; ++col)
                {
                ss << std::fixed << std::setprecision ( 3 ) << std::setw ( 8 )
                    << At ( row, col ) << " ";
                }
            ss << "\n";
            }
        CE_CORE_DEBUG ( "{}", ss.str () );
        }

    bool Matrix4::IsValid () const
        {
        for (size_t i = 0; i < 16; ++i)
            {
            if (!std::isfinite ( elements[ i ] ))
                return false;
            }
        return true;
        }


    Matrix4 Matrix4::operator+( const Matrix4 & other ) const {
        Matrix4 result;
        for (size_t i = 0; i < 16; ++i)
            {
            result.elements[ i ] = elements[ i ] + other.elements[ i ];
            }
        return result;
        }

    Matrix4 Matrix4::operator-( const Matrix4 & other ) const {
        Matrix4 result;
        for (size_t i = 0; i < 16; ++i)
            {
            result.elements[ i ] = elements[ i ] - other.elements[ i ];
            }
        return result;
        }

    Matrix4 Matrix4::operator*( const Matrix4 & other ) const {
        Matrix4 result;
        for (int row = 0; row < 4; ++row)
            {
            for (int col = 0; col < 4; ++col)
                {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                    {
// this[row][k] * other[k][col]
                    sum += At ( row, k ) * other.At ( k, col );
                    }
                result.At ( row, col ) = sum;
                }
            }
        return result;
        }

    Matrix4 Matrix4::operator*( float scalar ) const {
        Matrix4 result;
        for (size_t i = 0; i < 16; ++i)
            {
            result.elements[ i ] = elements[ i ] * scalar;
            }
        return result;
        }

    Vector4 Matrix4::operator*( const Vector4 & vector ) const {
       // ПРАВИЛЬНАЯ индексация для column-major матриц
        return Vector4 (
            elements[ 0 ] * vector.x + elements[ 4 ] * vector.y + elements[ 8 ] * vector.z + elements[ 12 ] * vector.w,
            elements[ 1 ] * vector.x + elements[ 5 ] * vector.y + elements[ 9 ] * vector.z + elements[ 13 ] * vector.w,
            elements[ 2 ] * vector.x + elements[ 6 ] * vector.y + elements[ 10 ] * vector.z + elements[ 14 ] * vector.w,
            elements[ 3 ] * vector.x + elements[ 7 ] * vector.y + elements[ 11 ] * vector.z + elements[ 15 ] * vector.w
        );
        }

    Matrix4 & Matrix4::operator+=( const Matrix4 & other ) {
        for (size_t i = 0; i < 16; ++i)
            {
            elements[ i ] += other.elements[ i ];
            }
        return *this;
        }

    Matrix4 & Matrix4::operator-=( const Matrix4 & other ) {
        for (size_t i = 0; i < 16; ++i)
            {
            elements[ i ] -= other.elements[ i ];
            }
        return *this;
        }

    Matrix4 & Matrix4::operator*=( const Matrix4 & other ) {
        *this = *this * other;
        return *this;
        }

    Matrix4 & Matrix4::operator*=( float scalar ) {
        for (size_t i = 0; i < 16; ++i)
            {
            elements[ i ] *= scalar;
            }
        return *this;
        }

    bool Matrix4::operator==( const Matrix4 & other ) const {
        for (size_t i = 0; i < 16; ++i)
            {
            if (std::abs ( elements[ i ] - other.elements[ i ] ) > std::numeric_limits<float>::epsilon ())
                {
                return false;
                }
            }
        return true;
        }

    bool Matrix4::operator!=( const Matrix4 & other ) const {
        return !( *this == other );
        }

    Matrix4 Matrix4::Transposed () const {
        Matrix4 result;
        for (size_t row = 0; row < 4; ++row)
            {
            for (size_t col = 0; col < 4; ++col)
                {
                result.At ( col, row ) = At ( row, col );
                }
            }
        return result;
        }

    void Matrix4::Transpose () {
        *this = Transposed ();
        }

    float Matrix4::Determinant () const {
        // Simplified 4x4 determinant calculation
        float det = 0.0f;
        det += elements[ 0 ] * ( elements[ 5 ] * ( elements[ 10 ] * elements[ 15 ] - elements[ 11 ] * elements[ 14 ] ) -
                                 elements[ 6 ] * ( elements[ 9 ] * elements[ 15 ] - elements[ 11 ] * elements[ 13 ] ) +
                                 elements[ 7 ] * ( elements[ 9 ] * elements[ 14 ] - elements[ 10 ] * elements[ 13 ] ) );

        det -= elements[ 1 ] * ( elements[ 4 ] * ( elements[ 10 ] * elements[ 15 ] - elements[ 11 ] * elements[ 14 ] ) -
                                 elements[ 6 ] * ( elements[ 8 ] * elements[ 15 ] - elements[ 11 ] * elements[ 12 ] ) +
                                 elements[ 7 ] * ( elements[ 8 ] * elements[ 14 ] - elements[ 10 ] * elements[ 12 ] ) );

        det += elements[ 2 ] * ( elements[ 4 ] * ( elements[ 9 ] * elements[ 15 ] - elements[ 11 ] * elements[ 13 ] ) -
                                 elements[ 5 ] * ( elements[ 8 ] * elements[ 15 ] - elements[ 11 ] * elements[ 12 ] ) +
                                 elements[ 7 ] * ( elements[ 8 ] * elements[ 13 ] - elements[ 9 ] * elements[ 12 ] ) );

        det -= elements[ 3 ] * ( elements[ 4 ] * ( elements[ 9 ] * elements[ 14 ] - elements[ 10 ] * elements[ 13 ] ) -
                                 elements[ 5 ] * ( elements[ 8 ] * elements[ 14 ] - elements[ 10 ] * elements[ 12 ] ) +
                                 elements[ 6 ] * ( elements[ 8 ] * elements[ 13 ] - elements[ 9 ] * elements[ 12 ] ) );

        return det;
        }

    Matrix4 Matrix4::Identity () {
        return Matrix4 ( 1.0f );
        }

    Matrix4 Matrix4::Translation ( const Vector3 & translation ) {
        Matrix4 result ( 1.0f );
        result.elements[ 12 ] = translation.x;
        result.elements[ 13 ] = translation.y;
        result.elements[ 14 ] = translation.z;
        return result;
        }

    Matrix4 Matrix4::RotationX ( float angle ) {
        Matrix4 result ( 1.0f );
        float cosA = std::cos ( angle );
        float sinA = std::sin ( angle );

        result.elements[ 5 ] = cosA;
        result.elements[ 6 ] = sinA;
        result.elements[ 9 ] = -sinA;
        result.elements[ 10 ] = cosA;

        return result;
        }

    Matrix4 Matrix4::RotationY ( float angle ) {
        Matrix4 result ( 1.0f );
        float cosA = std::cos ( angle );
        float sinA = std::sin ( angle );

        result.elements[ 0 ] = cosA;
        result.elements[ 2 ] = -sinA;
        result.elements[ 8 ] = sinA;
        result.elements[ 10 ] = cosA;

        return result;
        }

    Matrix4 Matrix4::RotationZ ( float angle ) {
        Matrix4 result ( 1.0f );
        float cosA = std::cos ( angle );
        float sinA = std::sin ( angle );

        result.elements[ 0 ] = cosA;
        result.elements[ 1 ] = sinA;
        result.elements[ 4 ] = -sinA;
        result.elements[ 5 ] = cosA;

        return result;
        }

    Matrix4 Matrix4::Rotation ( const Vector3 & axis, float angle ) {
        // Create rotation matrix from axis and angle
        Vector3 normalizedAxis = axis.Normalized ();
        float cosA = std::cos ( angle );
        float sinA = std::sin ( angle );
        float oneMinusCosA = 1.0f - cosA;

        float x = normalizedAxis.x;
        float y = normalizedAxis.y;
        float z = normalizedAxis.z;

        Matrix4 result;
        result.elements[ 0 ] = cosA + x * x * oneMinusCosA;
        result.elements[ 1 ] = y * x * oneMinusCosA + z * sinA;
        result.elements[ 2 ] = z * x * oneMinusCosA - y * sinA;

        result.elements[ 4 ] = x * y * oneMinusCosA - z * sinA;
        result.elements[ 5 ] = cosA + y * y * oneMinusCosA;
        result.elements[ 6 ] = z * y * oneMinusCosA + x * sinA;

        result.elements[ 8 ] = x * z * oneMinusCosA + y * sinA;
        result.elements[ 9 ] = y * z * oneMinusCosA - x * sinA;
        result.elements[ 10 ] = cosA + z * z * oneMinusCosA;

        result.elements[ 15 ] = 1.0f;

        return result;
        }

    Matrix4 Matrix4::Scale ( const Vector3 & scale ) {
        Matrix4 result ( 1.0f );
        result.elements[ 0 ] = scale.x;
        result.elements[ 5 ] = scale.y;
        result.elements[ 10 ] = scale.z;
        return result;
        }

    Matrix4 Matrix4::Orthographic ( float left, float right, float bottom, float top, float near, float far ) {
        Matrix4 result ( 1.0f );

        result.elements[ 0 ] = 2.0f / ( right - left );
        result.elements[ 5 ] = 2.0f / ( top - bottom );
        result.elements[ 10 ] = -2.0f / ( far - near );

        result.elements[ 12 ] = -( right + left ) / ( right - left );
        result.elements[ 13 ] = -( top + bottom ) / ( top - bottom );
        result.elements[ 14 ] = -( far + near ) / ( far - near );

        return result;
        }

    Matrix4 Matrix4::Perspective ( float fov, float aspect, float near, float far ) {
        Matrix4 result;

        float tanHalfFov = std::tan ( fov / 2.0f );
        float range = near - far;

        result.elements[ 0 ] = 1.0f / ( aspect * tanHalfFov );
        result.elements[ 5 ] = 1.0f / tanHalfFov;
        result.elements[ 10 ] = ( -near - far ) / range;
        result.elements[ 11 ] = 1.0f;
        result.elements[ 14 ] = ( 2.0f * far * near ) / range;
        result.elements[ 15 ] = 0.0f;

        return result;
        }

    Matrix4 Matrix4::LookAt ( const Vector3 & eye, const Vector3 & target, const Vector3 & up ) {
        Vector3 zAxis = ( eye - target ).Normalized ();
        Vector3 xAxis = up.Cross ( zAxis ).Normalized ();
        Vector3 yAxis = zAxis.Cross ( xAxis );

        Matrix4 result ( 1.0f );

        result.elements[ 0 ] = xAxis.x;
        result.elements[ 1 ] = yAxis.x;
        result.elements[ 2 ] = zAxis.x;

        result.elements[ 4 ] = xAxis.y;
        result.elements[ 5 ] = yAxis.y;
        result.elements[ 6 ] = zAxis.y;

        result.elements[ 8 ] = xAxis.z;
        result.elements[ 9 ] = yAxis.z;
        result.elements[ 10 ] = zAxis.z;

        result.elements[ 12 ] = -xAxis.Dot ( eye );
        result.elements[ 13 ] = -yAxis.Dot ( eye );
        result.elements[ 14 ] = -zAxis.Dot ( eye );

        return result;
        }

    Matrix4 Matrix4::Rotate ( const Quaternion & rotation ) {
        return rotation.ToMatrix ();
        }

    Matrix4 Matrix4::Inverted () const {
        // Временная реализация - возвращаем единичную матрицу
        return Identity ();
        }

    bool Matrix4::Invert () {
        // Временная реализация
        *this = Identity ();
        return true;
        }

        // Static constants
    const Matrix4 Matrix4::Zero = Matrix4 ( 0.0f );
    const Matrix4 Matrix4::IdentityMatrix = Matrix4 ( 1.0f );

    } // namespace ChudEngine::Math