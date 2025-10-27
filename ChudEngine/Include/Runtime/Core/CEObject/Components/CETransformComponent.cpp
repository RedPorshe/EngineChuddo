#include "Core/CEObject/Components/CETransformComponent.hpp"
#include "Utils/Logger.hpp"
#include "Math/MathUtils.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Math/Quaternion.hpp"
#include "Math/MathFunctions.hpp" 
#include <cmath>

namespace CE
    {
    CETransformComponent::CETransformComponent ()
        {
        SetName ( "CETransformComponent" );
        CE_DEBUG ( "CETransformComponent '{}' created", GetName () );
        }

    Math::Vector3 CETransformComponent::GetWorldPosition () const {
        Math::Matrix4 worldTransform = GetWorldTransform ();
        // Извлекаем позицию из колонки 3 (индексы 12, 13, 14)
        return Math::Vector3 ( worldTransform[ 12 ], worldTransform[ 13 ], worldTransform[ 14 ] );
        }

    Math::Quaternion CETransformComponent::GetRotationQuaternion () const
        {
            // Convert Euler angles (degrees) to quaternion
        float pitch = Rotation.x * ( 3.14159f / 180.0f );
        float yaw = Rotation.y * ( 3.14159f / 180.0f );
        float roll = Rotation.z * ( 3.14159f / 180.0f );

        float cy = cos ( yaw * 0.5f );
        float sy = sin ( yaw * 0.5f );
        float cp = cos ( pitch * 0.5f );
        float sp = sin ( pitch * 0.5f );
        float cr = cos ( roll * 0.5f );
        float sr = sin ( roll * 0.5f );

        Math::Quaternion q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;

        return q;
        }

    void CETransformComponent::SetPosition ( const Math::Vector3 & NewPosition )
        {
        Position = NewPosition;
        MarkDirty ();
        CE_DEBUG ( "Transform '{}' position set to ({}, {}, {})",
                   GetName (), Position.x, Position.y, Position.z );
        }

    void CETransformComponent::SetRotation ( const Math::Vector3 & NewRotation )
        {
        Rotation = NewRotation;
        MarkDirty ();
        CE_DEBUG ( "Transform '{}' rotation set to ({}, {}, {})",
                   GetName (), Rotation.x, Rotation.y, Rotation.z );
        }

    void CETransformComponent::SetScale ( const Math::Vector3 & NewScale )
        {
        Scale = NewScale;
        MarkDirty ();
        CE_DEBUG ( "Transform '{}' scale set to ({}, {}, {})",
                   GetName (), Scale.x, Scale.y, Scale.z );
        }

    void CETransformComponent::Translate ( const Math::Vector3 & Translation )
        {
        Position = Position + Translation;
        MarkDirty ();
        }

    void CETransformComponent::Rotate ( const Math::Vector3 & RotationDelta )
        {
        Rotation = Rotation + RotationDelta;
        MarkDirty ();
        }

    void CETransformComponent::LookAt ( const Math::Vector3 & Target )
        {
            // Simple look-at implementation
        Math::Vector3 direction = Math::Normalize ( Target - GetWorldPosition () );

        // Calculate rotation from direction vector
        // This is a simplified version - you might want a more robust implementation
        float yaw = atan2 ( direction.x, direction.z );
        float pitch = asin ( -direction.y );

        Rotation = Math::Vector3 ( pitch * ( 180.0f / 3.14159f ),
                                   yaw * ( 180.0f / 3.14159f ),
                                   0.0f );
        MarkDirty ();
        }

    Math::Matrix4 CETransformComponent::GetLocalTransform () const {
        if (bTransformDirty)
            {
            UpdateTransform ();

            // ОТЛАДОЧНЫЙ ВЫВОД
            CE_DEBUG ( "=== Local Transform Update ===" );
            CE_DEBUG ( "Position: ({}, {}, {})", Position.x, Position.y, Position.z );
            CE_DEBUG ( "Rotation: ({}, {}, {})", Rotation.x, Rotation.y, Rotation.z );
            CE_DEBUG ( "Scale: ({}, {}, {})", Scale.x, Scale.y, Scale.z );
          
            CE_DEBUG ( "=============================" );
            }
        return CachedLocalTransform;
        }

    Math::Matrix4 CETransformComponent::GetWorldTransform () const {
        Math::Matrix4 local = GetLocalTransform ();

        if (Parent)
            {
            Math::Matrix4 parentWorld = Parent->GetWorldTransform ();
            Math::Matrix4 result = parentWorld * local;

            // ОТЛАДОЧНЫЙ ВЫВОД
            CE_DEBUG ( "=== World Transform ===" );
            CE_DEBUG ( "Actor: {}", GetName () );
            
           
            
            CE_DEBUG ( "======================" );

            return result;
            }

        return local;
        }

    void CETransformComponent::SetParent ( CETransformComponent * NewParent )
        {
        if (Parent == NewParent) return;

        // Remove from current parent
        if (Parent)
            {
            Parent->RemoveChild ( this );
            }

            // Set new parent
        Parent = NewParent;

        // Add to new parent
        if (Parent)
            {
            Parent->AddChild ( this );
            }

        MarkDirty ();
        }

    Math::Vector3 CETransformComponent::GetForward () const
        {
        Math::Quaternion rot = GetRotationQuaternion ();
        Math::Vector3 forward ( 0.0f, 0.0f, 1.0f );
        return rot.Rotate ( forward ); // Используй Rotate вместо operator*
        }

    Math::Vector3 CETransformComponent::GetRight () const
        {
        Math::Quaternion rot = GetRotationQuaternion ();
        Math::Vector3 right ( 1.0f, 0.0f, 0.0f );
        return rot.Rotate ( right ); // Используй Rotate вместо operator*
        }

    Math::Vector3 CETransformComponent::GetUp () const
        {
        Math::Quaternion rot = GetRotationQuaternion ();
        Math::Vector3 up ( 0.0f, 1.0f, 0.0f );
        return rot.Rotate ( up ); // Используй Rotate вместо operator*
        }

    void CETransformComponent::MarkDirty ()
        {
        bTransformDirty = true;
        OnTransformChanged ();

        // Mark children as dirty too
        for (auto * child : Children)
            {
            if (child)
                {
                child->MarkDirty ();
                }
            }
        }

    void CETransformComponent::OnTransformChanged ()
        {
            // Can be overridden by derived classes
        }

    void CETransformComponent::UpdateTransform () const {
        Math::Matrix4 translation = Math::Matrix4::Translation ( Position );
        Math::Quaternion rotQuat = GetRotationQuaternion ();
        Math::Matrix4 rotation = rotQuat.ToMatrix ();
        Math::Matrix4 scale = Math::Matrix4::Scale ( Scale );

        // ИСПРАВЛЕНИЕ: Правильный порядок - Translation * Rotation * Scale
        CachedLocalTransform = translation * rotation * scale;

        // ОТЛАДОЧНЫЙ ВЫВОД
        CE_DEBUG ( "=== Transform Update: {} ===", GetName () );
        CE_DEBUG ( "Position: ({:.1f}, {:.1f}, {:.1f})", Position.x, Position.y, Position.z );
        CE_DEBUG ( "Scale: ({:.1f}, {:.1f}, {:.1f})", Scale.x, Scale.y, Scale.z );

        // Проверка результата
        Math::Vector4 testPoint ( 0, 0, 0, 1 ); // Локальное начало
        Math::Vector4 worldPoint = CachedLocalTransform * testPoint;
        CE_DEBUG ( "Local (0,0,0) -> World ({:.1f}, {:.1f}, {:.1f})",
                   worldPoint.x, worldPoint.y, worldPoint.z );

          // Детальная диагностика матриц
        CachedLocalTransform.DebugPrint ( "Local Transform" );

        bTransformDirty = false;
        }

    void CETransformComponent::AddChild ( CETransformComponent * Child )
        {
        if (Child && std::find ( Children.begin (), Children.end (), Child ) == Children.end ())
            {
            Children.push_back ( Child );
            }
        }

    void CETransformComponent::RemoveChild ( CETransformComponent * Child )
        {
        auto it = std::find ( Children.begin (), Children.end (), Child );
        if (it != Children.end ())
            {
            Children.erase ( it );
            }
        }
    }