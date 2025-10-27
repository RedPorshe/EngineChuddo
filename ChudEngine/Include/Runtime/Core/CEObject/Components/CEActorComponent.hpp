#pragma once
#include "Core/CEObject/CEObject.hpp"


namespace CE
    {
    class CEActor;

    class CEActorComponent : public CEObject
        {
        public:
            CEActorComponent ();
            virtual ~CEActorComponent ();

            // Владелец компонента
            CEActor * GetOwner () const { return Owner; }
            void SetOwner ( CEActor * NewOwner ) { Owner = NewOwner; }

            // Активность компонента
            bool IsActive () const { return bIsActive; }
            void SetActive ( bool bNewActive ) { bIsActive = bNewActive; }

            // Виртуальные методы
            virtual void Initialize ();
            virtual void BeginPlay ();
            virtual void TickComponent ( float DeltaTime );
            virtual void OnComponentDestroyed ();

        protected:
            CEActor * Owner = nullptr;
            bool bIsActive = true;
        };
    }