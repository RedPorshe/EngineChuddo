#pragma once

#include "Core/CEObject/CEObject.hpp"

namespace CE
    {
    class CEActor;

    class CEComponent : public CEObject
        {
        public:
            CEComponent ();
            virtual ~CEComponent () = default;

            // Lifecycle
            virtual void BeginPlay () override;
            virtual void Tick ( float DeltaTime ) override;
            
            // Owner management
            CEActor * GetOwner () const { return Owner; }
            void SetOwner ( CEActor * InOwner ) { Owner = InOwner; }

            // Activity
            bool IsActive () const { return bIsActive; }
            void SetActive ( bool bActive ) { bIsActive = bActive; }

        protected:
            CEActor * Owner = nullptr;
            bool bIsActive = true;
        };
    }