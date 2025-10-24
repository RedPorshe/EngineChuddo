// Source/Runtime/Core/CEObject/CEEvents.hpp
#pragma once
#include "Framework/Math/Vector.hpp"
#include "CEEvent.hpp"  // Добавляем include базового класса
#include "CEActor.hpp"

namespace CE
    {
        // Базовые игровые события
    class CEActorDestroyedEvent : public CEEvent
        {
        public:
            CEActor * DestroyedActor;

            CEActorDestroyedEvent ( CEActor * Actor ) : DestroyedActor ( Actor )
                {
                SetName ( "ActorDestroyedEvent" );
                }
        };

    class CEActorSpawnedEvent : public CEEvent
        {
        public:
            CEActor * SpawnedActor;

            CEActorSpawnedEvent ( CEActor * Actor ) : SpawnedActor ( Actor )
                {
                SetName ( "ActorSpawnedEvent" );
                }
        };

    class CEHitEvent : public CEEvent
        {
        public:
            CEActor * Instigator;
            CEActor * Target;
            Math::Vector3 HitLocation;
            float Damage;

            CEHitEvent ( CEActor * InstigatorActor, CEActor * TargetActor, const Math::Vector3 & Location, float DamageAmount )
                : Instigator ( InstigatorActor ), Target ( TargetActor ), HitLocation ( Location ), Damage ( DamageAmount )
                {
                SetName ( "HitEvent" );
                }
        };

    class CEBeginOverlapEvent : public CEEvent
        {
        public:
            CEActor * OverlappingActor;
            CEActor * OtherActor;

            CEBeginOverlapEvent ( CEActor * Actor1, CEActor * Actor2 )
                : OverlappingActor ( Actor1 ), OtherActor ( Actor2 )
                {
                SetName ( "BeginOverlapEvent" );
                }
        };

    class CEEndOverlapEvent : public CEEvent
        {
        public:
            CEActor * OverlappingActor;
            CEActor * OtherActor;

            CEEndOverlapEvent ( CEActor * Actor1, CEActor * Actor2 )
                : OverlappingActor ( Actor1 ), OtherActor ( Actor2 )
                {
                SetName ( "EndOverlapEvent" );
                }
        };
    }