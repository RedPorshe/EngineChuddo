// Runtime/Core/CEObject/Components/CETransformComponent.cpp
#include "CETransformComponent.hpp"
#include "Core/Logger.h"

namespace CE
    {
    CETransformComponent::CETransformComponent ()
        {
        SetName ( "CETransformComponent" );
        CE_DEBUG ( "CETransformComponent '{}' created", GetName () );
        }

    Math::Vector3 CETransformComponent::GetWorldPosition () const
        {
        if (Parent)
            {
            Math::Matrix4 worldTransform = Parent->GetWorldTransform ();
            Math::Vector4 worldPos = worldTransform * Math::Vector4 ( Position, 1.0f );
            return Math::Vector3 ( worldPos.x, worldPos.y, worldPos.z );
            }
        return Position;
        }

    Math::Quaternion CETransformComponent::GetRotationQuaternion () const
        {
        return Math::Quaternion::FromEulerAngles ( Rotation * ( 3.14159f / 180.0f ) ); // Convert to radians
        }

    void CETransformComponent::SetPosition ( const Math::Vector3 & NewPosition )
        {
        if (Position != NewPosition)
            {
            Position = NewPosition;
            MarkDirty ();
           
            }
        }

    void CETransformComponent::SetRotation ( const Math::Vector3 & NewRotation )
        {
        if (Rotation != NewRotation)
            {
            Rotation = NewRotation;
            MarkDirty ();
            
            }
        }

    void CETransformComponent::SetScale ( const Math::Vector3 & NewScale )
        {
        if (Scale != NewScale)
            {
            Scale = NewScale;
            MarkDirty ();
            
            }
        }

    void CETransformComponent::Translate ( const Math::Vector3 & Translation )
        {
        Position += Translation;
        MarkDirty ();
        }

    void CETransformComponent::Rotate ( const Math::Vector3 & RotationDelta )
        {
        Rotation += RotationDelta;
        MarkDirty ();
        }

    void CETransformComponent::LookAt ( const Math::Vector3 & Target )
        {
        Math::Vector3 direction = ( Target - GetWorldPosition () ).Normalized ();

        // Calculate rotation from direction vector
        float pitch = std::asin ( -direction.y );
        float yaw = std::atan2 ( direction.x, direction.z );

        Rotation = Math::Vector3 (
            pitch * ( 180.0f / 3.14159f ),
            yaw * ( 180.0f / 3.14159f ),
            0.0f
        );
        MarkDirty ();
        }

    Math::Matrix4 CETransformComponent::GetLocalTransform () const
        {
        UpdateTransform ();
        return CachedLocalTransform;
        }

    Math::Matrix4 CETransformComponent::GetWorldTransform () const
        {
        UpdateTransform ();

        if (Parent)
            {
            return Parent->GetWorldTransform () * CachedLocalTransform;
            }

        return CachedWorldTransform;
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
        return rot.Rotate ( Math::Vector3 ( 0.0f, 0.0f, 1.0f ) ); // Z-forward
        }


    Math::Vector3 CETransformComponent::GetRight () const
        {
        Math::Quaternion rot = GetRotationQuaternion ();
        return rot.Rotate ( Math::Vector3 ( 1.0f, 0.0f, 0.0f ) ); // X-right
        }

    Math::Vector3 CETransformComponent::GetUp () const
        {
        Math::Quaternion rot = GetRotationQuaternion ();
        return rot.Rotate ( Math::Vector3 ( 0.0f, 1.0f, 0.0f ) ); // Y-up
        }

    void CETransformComponent::MarkDirty ()
        {
        bTransformDirty = true;

        // Mark all children as dirty too
        for (auto * child : Children)
            {
            child->MarkDirty ();
            }

        OnTransformChanged ();
        }

    void CETransformComponent::OnTransformChanged ()
        {
            // Can be overridden by derived classes to respond to transform changes
        }

    void CETransformComponent::UpdateTransform () const
        {
        if (!bTransformDirty) return;

        // Правильный порядок для SRT: Scale * Rotation * Translation
        Math::Matrix4 scale = Math::Matrix4::Scale ( Scale );
        Math::Matrix4 rotation = GetRotationQuaternion ().ToMatrix (); // Используем метод кватерниона
        Math::Matrix4 translation = Math::Matrix4::Translation ( Position );

        // Умножение в правильном порядке
        CachedLocalTransform = translation * rotation * scale;
        CachedWorldTransform = CachedLocalTransform;

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