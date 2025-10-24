// Source/Runtime/Core/CEObject/CESceneComponent.cpp

#include "Core/CEObject/Components/CESceneComponent.hpp"

namespace CE
    {
    CESceneComponent::CESceneComponent ()
        {
        SetName ( "CESceneComponent" );
        CE_DEBUG ( "CESceneComponent '{}' created", GetName () );
        }

    CESceneComponent::~CESceneComponent ()
        {
            // ����������� �� ��������
        DetachFromParent ();

        // ����������� ���� �����
        for (auto * Child : Children)
            {
            Child->Parent = nullptr;
            }
        Children.clear ();

        CE_DEBUG ( "CESceneComponent '{}' destroyed", GetName () );
        }

    void CESceneComponent::SetRelativeLocation ( const Math::Vector3 & NewLocation )
        {
        RelativeLocation = NewLocation;
        bTransformDirty = true;
        CE_DEBUG ( "CESceneComponent '{}' location set to ({}, {}, {})",
                   GetName (), NewLocation.x, NewLocation.y, NewLocation.z );
        }

    void CESceneComponent::SetRelativeRotation ( const Math::Quaternion & NewRotation )
        {
        RelativeRotation = NewRotation;
        bTransformDirty = true;
        }

    void CESceneComponent::SetRelativeScale ( const Math::Vector3 & NewScale )
        {
        RelativeScale = NewScale;
        bTransformDirty = true;
        }

    Math::Vector3 CESceneComponent::GetWorldLocation () const
        {
        if (Parent)
            {
                // ���������� ������� ������������� ��� ����������� ����� ��������
            CESceneComponent * nonConstThis = const_cast< CESceneComponent * >( this );
            Math::Matrix4 worldTransform = nonConstThis->GetWorldTransform ();
            return Math::Vector3 ( worldTransform.elements[ 12 ], worldTransform.elements[ 13 ], worldTransform.elements[ 14 ] );
            }
        return RelativeLocation;
        }

    Math::Quaternion CESceneComponent::GetWorldRotation () const
        {
        if (Parent)
            {
            return Parent->GetWorldRotation () * RelativeRotation;
            }
        return RelativeRotation;
        }

    Math::Vector3 CESceneComponent::GetWorldScale () const
        {
        if (Parent)
            {
            Math::Vector3 parentScale = Parent->GetWorldScale ();
            return Math::Vector3 (
                parentScale.x * RelativeScale.x,
                parentScale.y * RelativeScale.y,
                parentScale.z * RelativeScale.z
            );
            }
        return RelativeScale;
        }

    Math::Matrix4 CESceneComponent::GetWorldTransform ()
        {
        if (bTransformDirty)
            {
            CE_DEBUG ( "Updating world transform for '{}'", GetName () );

            // ������� identity �������
            CachedWorldTransform = Math::Matrix4::Identity ();

            // ��������� �������������: Scale -> Rotate -> Translate
            Math::Matrix4 scaleMat = Math::Matrix4::Scale ( RelativeScale );
            Math::Matrix4 rotationMat = RelativeRotation.ToMatrix ();
            Math::Matrix4 translationMat = Math::Matrix4::Translation ( RelativeLocation );

            // �����������: T * R * S
            CachedWorldTransform = translationMat * rotationMat * scaleMat;

            // �������� �� ������������ ������������� ���� ���� ��������
            if (Parent)
                {
                CachedWorldTransform = Parent->GetWorldTransform () * CachedWorldTransform;
                }

            bTransformDirty = false;
            }
        return CachedWorldTransform;
        }

    void CESceneComponent::AttachTo ( CESceneComponent * NewParent )
        {
        if (Parent == NewParent) return;

        // ����������� �� �������� ��������
        DetachFromParent ();

        // ������������ � ������ ��������
        Parent = NewParent;
        if (Parent)
            {
            Parent->Children.push_back ( this );
            bTransformDirty = true;
            CE_DEBUG ( "CESceneComponent '{}' attached to '{}'", GetName (), Parent->GetName () );
            }
        }

    void CESceneComponent::DetachFromParent ()
        {
        if (Parent)
            {
                // ������� ���� �� ����� ��������
            auto & parentChildren = Parent->Children;
            parentChildren.erase (
                std::remove ( parentChildren.begin (), parentChildren.end (), this ),
                parentChildren.end ()
            );

            CE_DEBUG ( "CESceneComponent '{}' detached from '{}'", GetName (), Parent->GetName () );
            Parent = nullptr;
            bTransformDirty = true;
            }
        }

    void CESceneComponent::BeginPlay ()
        {
        CEActorComponent::BeginPlay ();
        CE_DEBUG ( "CESceneComponent '{}' BeginPlay", GetName () );
        }

    void CESceneComponent::TickComponent ( float DeltaTime )
        {
        CEActorComponent::TickComponent ( DeltaTime );
        // ����� ��������� ���-�� ��������� � ��������������
        }

    void CESceneComponent::UpdateTransform ()
        {
        bTransformDirty = true;
        // �������� ����� ��� dirty ����
        for (auto * Child : Children)
            {
            Child->UpdateTransform ();
            }
        }
    }