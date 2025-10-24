// Source/Runtime/Core/CEObject/CESceneComponent.hpp
#pragma once

#include "CEActorComponent.hpp"
#include "Math/Vector.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Matrix.hpp"
#include <vector>

namespace CE
    {
    class CESceneComponent : public CEActorComponent
        {
        public:
            CESceneComponent ();
            virtual ~CESceneComponent ();

            // Локальные трансформации относительно родителя
            Math::Vector3 GetRelativeLocation () const { return RelativeLocation; }
            Math::Quaternion GetRelativeRotation () const { return RelativeRotation; }
            Math::Vector3 GetRelativeScale () const { return RelativeScale; }

            void SetRelativeLocation ( const Math::Vector3 & NewLocation );
            void SetRelativeRotation ( const Math::Quaternion & NewRotation );
            void SetRelativeScale ( const Math::Vector3 & NewScale );

            // Мировые трансформации
            Math::Vector3 GetWorldLocation () const;
            Math::Quaternion GetWorldRotation () const;
            Math::Vector3 GetWorldScale () const;
            Math::Matrix4 GetWorldTransform (); // Убрали const!

            // Иерархия
            void AttachTo ( CESceneComponent * NewParent );
            void DetachFromParent ();
            CESceneComponent * GetParent () const { return Parent; }
            const std::vector<CESceneComponent *> & GetChildren () const { return Children; }

            // Переопределенные методы
            virtual void BeginPlay () override;
            virtual void TickComponent ( float DeltaTime ) override;

        protected:
            Math::Vector3 RelativeLocation;
            Math::Quaternion RelativeRotation;
            Math::Vector3 RelativeScale;

            CESceneComponent * Parent = nullptr;
            std::vector<CESceneComponent *> Children;

        private:
            void UpdateTransform ();
            mutable Math::Matrix4 CachedWorldTransform; // Добавили mutable
            mutable bool bTransformDirty = true;        // Добавили mutable
        };
    }