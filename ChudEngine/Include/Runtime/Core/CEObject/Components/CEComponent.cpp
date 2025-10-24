// Runtime/Core/CEObject/CEComponent.cpp
#include "Core/CEObject/Components/CEComponent.hpp"
#include "Core/CEObject/CEActor.hpp"

namespace CE
    {
    CEComponent::CEComponent ()        
        {
        Name = "CEComponent";
        CE_DEBUG ( "CEComponent '{}' created (ID: {})", Name, UniqueID );
        }

    void CEComponent::BeginPlay ()
        {
        CEObject::BeginPlay ();
        CE_DEBUG ( " CEComponent BeginPlay: {}", Name );
        }

    void CEComponent::Tick ( float DeltaTime )
        {
        CEObject::Tick ( DeltaTime );
        }

   
    }