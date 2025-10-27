#pragma once
#include "Core/CEObject/Components/CEComponent.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Math/Quaternion.hpp"

namespace CE
    {
    class CETransformComponent : public CEComponent
        {
        public:
            CETransformComponent ();
            virtual ~CETransformComponent () = default;

            // Transform properties
            Math::Vector3 GetPosition () const { return Position; }
            Math::Vector3 GetRotation () const { return Rotation; }
            Math::Vector3 GetScale () const { return Scale; }
            Math::Vector3 GetWorldPosition () const;
            Math::Quaternion GetRotationQuaternion () const;

            void SetPosition ( const Math::Vector3 & NewPosition );
            void SetRotation ( const Math::Vector3 & NewRotation );
            void SetScale ( const Math::Vector3 & NewScale );
            void SetScale ( float UniformScale ) { SetScale ( Math::Vector3 ( UniformScale ) ); }

            // Transform operations
            void Translate ( const Math::Vector3 & Translation );
            void Rotate ( const Math::Vector3 & RotationDelta );
            void LookAt ( const Math::Vector3 & Target );

            // Matrix operations
            Math::Matrix4 GetLocalTransform () const;
            Math::Matrix4 GetWorldTransform () const;

            // Hierarchy
            void SetParent ( CETransformComponent * NewParent );
            CETransformComponent * GetParent () const { return Parent; }
            const std::vector<CETransformComponent *> & GetChildren () const { return Children; }

            // Direction vectors
            Math::Vector3 GetForward () const;
            Math::Vector3 GetRight () const;
            Math::Vector3 GetUp () const;

            // Dirty flag for optimization
            bool IsDirty () const { return bTransformDirty; }
            void MarkDirty ();

        protected:
            virtual void OnTransformChanged ();

        private:
            Math::Vector3 Position = Math::Vector3 ( 0.0f, 0.0f, 0.0f );
            Math::Vector3 Rotation = Math::Vector3 ( 0.0f, 0.0f, 0.0f ); // Euler angles in degrees
            Math::Vector3 Scale = Math::Vector3 ( 1.0f, 1.0f, 1.0f );

            // Hierarchy
            CETransformComponent * Parent = nullptr;
            std::vector<CETransformComponent *> Children;

            // Cached matrices
            mutable Math::Matrix4 CachedLocalTransform;
            mutable Math::Matrix4 CachedWorldTransform;
            mutable bool bTransformDirty = true;

            void UpdateTransform () const;
            void AddChild ( CETransformComponent * Child );
            void RemoveChild ( CETransformComponent * Child );
        };
    }