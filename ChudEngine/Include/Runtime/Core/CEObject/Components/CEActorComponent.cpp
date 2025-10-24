#include "Core/CEObject/Components/CEActorComponent.hpp"
#include "Core/CEObject/CEActor.hpp"



namespace CE
    {
    CEActorComponent::CEActorComponent ()
        {
        SetName ( "CEActorComponent" ); // Устанавливаем имя по умолчанию
        CE_DEBUG ( "CEActorComponent '{}' created (ID: {})", GetName (), GetUniqueID () );
        }

    CEActorComponent::~CEActorComponent ()
        {
        CE_DEBUG ( "CEActorComponent destroyed: {}", GetName () );
        }

    void CEActorComponent::Initialize ()
        {
        CE_DEBUG ( "CEActorComponent Initialize: {}", GetName () );
        }

    void CEActorComponent::BeginPlay ()
        {
        CEObject::BeginPlay ();
        CE_DEBUG ( "CEActorComponent BeginPlay: {} (Owner: {})", GetName (), Owner ? Owner->GetName () : "None" );
        }

    void CEActorComponent::TickComponent ( float DeltaTime )
        {
        if (bIsActive)
            {
                // Базовая реализация пуста
            }
        }

    void CEActorComponent::OnComponentDestroyed ()
        {
        CE_DEBUG ( "CEActorComponent OnComponentDestroyed: {}", GetName () );
        }
    }