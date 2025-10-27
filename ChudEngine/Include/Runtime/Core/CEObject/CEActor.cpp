#include "Core/CEObject/CEActor.hpp"

namespace CE
    {
    CEActor::CEActor ( const std::string & ActorName ) : CEObject ( ActorName )
        {       
        TransformComponent = CreateComponent<CETransformComponent> ();
        TransformComponent->SetName ( Name + "_Transform" );

        CE_DEBUG ( " CEActor '{}' created (ID: {}) with transform", Name, UniqueID );
        bInitialized = true; 
        }

    CEActor::~CEActor ()
        {
        bPendingKill = true; 
        CE_DEBUG ( " CEActor '{}' destroyed", Name );
        }

    void CEActor::BeginPlay ()
        {
        CEObject::BeginPlay ();
        bInitialized = true;

        // Initialize all components
        for (auto & Component : Components)
            {
            if (Component->IsActive ())
                {
                Component->BeginPlay ();
                }
            }
        }

    void CEActor::Tick ( float DeltaTime )
        {
        CEObject::Tick ( DeltaTime );

        for (auto & Component : Components)
            {
            if (Component->IsActive ())
                {
                Component->Tick ( DeltaTime );
                }
            }
        }

    void CEActor::RegisterComponent ( CEComponent * Component )
        {
        std::type_index type = typeid( *Component );
        ComponentMap[ type ].PushBack ( Component );
        CE_DEBUG ( "Registered component of type {} for actor {}", type.name (), Name );
        }
    }