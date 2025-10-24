#include "Framework/Math/Matrix.hpp"
#include "Framework/Core/Logger.h"



void DebugMatrices ( const CE::Math::Matrix4 & model, const CE::Math::Matrix4 & view, const CE::Math::Matrix4 & proj ) {
    CE_CORE_DEBUG ( "=== Matrix Debug ===" );

    CE_CORE_DEBUG ( "Model Matrix (column-major):" );
    for (int row = 0; row < 4; ++row)
        {
        CE_CORE_DEBUG ( "  [{:.3f}, {:.3f}, {:.3f}, {:.3f}]",
                        model.At ( row, 0 ), model.At ( row, 1 ), model.At ( row, 2 ), model.At ( row, 3 ) );
        }

    CE_CORE_DEBUG ( "View Matrix (column-major):" );
    for (int row = 0; row < 4; ++row)
        {
        CE_CORE_DEBUG ( "  [{:.3f}, {:.3f}, {:.3f}, {:.3f}]",
                        view.At ( row, 0 ), view.At ( row, 1 ), view.At ( row, 2 ), view.At ( row, 3 ) );
        }

    CE_CORE_DEBUG ( "Projection Matrix (column-major):" );
    for (int row = 0; row < 4; ++row)
        {
        CE_CORE_DEBUG ( "  [{:.3f}, {:.3f}, {:.3f}, {:.3f}]",
                        proj.At ( row, 0 ), proj.At ( row, 1 ), proj.At ( row, 2 ), proj.At ( row, 3 ) );
        }

        // Проверка ключевых элементов
    CE_CORE_DEBUG ( "Key elements - Model translation: [{:.3f}, {:.3f}, {:.3f}]",
                    model.At ( 3, 0 ), model.At ( 3, 1 ), model.At ( 3, 2 ) );
    CE_CORE_DEBUG ( "Key elements - View translation: [{:.3f}, {:.3f}, {:.3f}]",
                    view.At ( 3, 0 ), view.At ( 3, 1 ), view.At ( 3, 2 ) );
    CE_CORE_DEBUG ( "Projection [2][3]: {:.3f} (should be 1.0 for perspective)", proj.At ( 2, 3 ) );
    }

void QuickMatrixCheck ( const CE::Math::Matrix4 & model, const CE::Math::Matrix4 & view, const CE::Math::Matrix4 & proj ) {
    CE_CORE_DEBUG ( "=== Quick Matrix Check ===" );

    // Проверяем что матрицы не нулевые
    bool modelValid = ( model.At ( 0, 0 ) != 0.0f || model.At ( 1, 1 ) != 0.0f || model.At ( 2, 2 ) != 0.0f );
    bool viewValid = ( view.At ( 0, 0 ) != 0.0f || view.At ( 1, 1 ) != 0.0f || view.At ( 2, 2 ) != 0.0f );
    bool projValid = ( proj.At ( 0, 0 ) != 0.0f || proj.At ( 1, 1 ) != 0.0f );

    CE_CORE_DEBUG ( "Model valid: {} (diagonal: {:.3f}, {:.3f}, {:.3f})",
                    modelValid, model.At ( 0, 0 ), model.At ( 1, 1 ), model.At ( 2, 2 ) );
    CE_CORE_DEBUG ( "View valid: {} (diagonal: {:.3f}, {:.3f}, {:.3f})",
                    viewValid, view.At ( 0, 0 ), view.At ( 1, 1 ), view.At ( 2, 2 ) );
    CE_CORE_DEBUG ( "Projection valid: {} (scale: {:.3f}, {:.3f})",
                    projValid, proj.At ( 0, 0 ), proj.At ( 1, 1 ) );

                // Проверка трансформации
    CE::Math::Vector4 testPoint ( 0.0f, 0.0f, 0.0f, 1.0f );
    auto transformed = proj * view * model * testPoint;
    CE_CORE_DEBUG ( "Test point [0,0,0,1] transformed to: [{:.3f}, {:.3f}, {:.3f}, {:.3f}]",
                    transformed.x, transformed.y, transformed.z, transformed.w );
    }

void CheckCommonMatrixProblems () {
    CE_CORE_DEBUG ( "=== Checking Common Matrix Problems ===" );

    // 1. Проверка идентичной матрицы
    auto identity = CE::Math::Matrix4::Identity ();
    CE_CORE_DEBUG ( "Identity matrix diagonal: {:.1f}, {:.1f}, {:.1f}, {:.1f}",
                    identity.At ( 0, 0 ), identity.At ( 1, 1 ), identity.At ( 2, 2 ), identity.At ( 3, 3 ) );

                // 2. Проверка создания простой трансформации
    auto translation = CE::Math::Matrix4::Translation ( CE::Math::Vector3 ( 1.0f, 2.0f, 3.0f ) );
    CE_CORE_DEBUG ( "Translation matrix: [{:.1f}, {:.1f}, {:.1f}]",
                    translation.At ( 3, 0 ), translation.At ( 3, 1 ), translation.At ( 3, 2 ) );

                // 3. Проверка проекционной матрицы
    auto perspective = CE::Math::Matrix4::Perspective (
        60.0f * 3.14159f / 180.0f,  // FOV в радианах
        1280.0f / 720.0f,           // Aspect ratio
        0.1f,                       // Near
        100.0f                      // Far
    );
    CE_CORE_DEBUG ( "Perspective matrix - [0][0]: {:.3f}, [1][1]: {:.3f}, [2][3]: {:.3f}",
                    perspective.At ( 0, 0 ), perspective.At ( 1, 1 ), perspective.At ( 2, 3 ) );
    }