#include "Core/CEObject/CEWorld.hpp"

#include "Core/Logger.h"
#include <algorithm>

namespace CE
    {
    CEWorld::CEWorld ( const std::string & WorldName ) 
        : CEObject(WorldName), TickManager ( new CETickManager (WorldName+" tickManager"))
        {       
        std::string safeName = GetName ();
        CE_DEBUG ( "CEWorld '{}' created", safeName );
        }

    CEWorld::~CEWorld ()
        {
        std::string safeName = GetName ();
        CE_DEBUG ( "CEWorld '{}' destructor started with {} actors", safeName, Actors.size () );

        // Уничтожаем всех акторов в обратном порядке
        for (auto it = Actors.rbegin (); it != Actors.rend (); ++it)
            {
            CEActor * Actor = *it;
            if (Actor)
                {
                std::string actorName = Actor->GetName ();
                Actor->Destroy ();
                delete Actor;
                CE_DEBUG ( "Destroyed actor: {}", actorName );
                }
            }
        Actors.clear ();

        // Очищаем pending акторы
        for (CEActor * Actor : PendingActors)
            {
            if (Actor)
                {
                std::string actorName = Actor->GetName ();
                delete Actor;
                CE_DEBUG ( "Cleaned pending actor: {}", actorName );
                }
            }
        PendingActors.clear ();

        for (CEActor * Actor : PendingKillActors)
            {
            if (Actor)
                {
                std::string actorName = Actor->GetName ();
                delete Actor;
                CE_DEBUG ( "Cleaned pending kill actor: {}", actorName );
                }
            }
        PendingKillActors.clear ();

        // Уничтожаем TickManager
        if (TickManager)
            {
            delete TickManager;
            TickManager = nullptr;
            }

        CE_DEBUG ( "CEWorld '{}' destroyed", safeName );
        }

    void CEWorld::SpawnActor ( CEActor * Actor )
        {
        if (Actor)
            {
            PendingActors.push_back ( Actor );
            std::string actorName = Actor->GetName ();
            CE_DEBUG ( "CEWorld: Actor '{}' scheduled for spawn", actorName );
            }
        }

    void CEWorld::DestroyActor ( CEActor * Actor )
        {
        if (Actor && std::find ( Actors.begin (), Actors.end (), Actor ) != Actors.end ())
            {
            PendingKillActors.push_back ( Actor );
            std::string actorName = Actor->GetName ();
            CE_DEBUG ( "CEWorld: Actor '{}' scheduled for destruction", actorName );
            }
        }

    void CEWorld::BeginPlay ()
        {
        CEObject::BeginPlay ();
        ProcessPendingSpawns ();
        std::string safeName = GetName ();
        CE_DEBUG ( "CEWorld '{}' BeginPlay with {} actors", safeName, Actors.size () );
        }

    void CEWorld::Tick ( float DeltaTime )
        {
        CEObject::Tick ( DeltaTime );

        // Обрабатываем отложенные операции ДО обновления тиков
        ProcessPendingSpawns ();
        ProcessPendingKills ();

        // Обновляем тик-менеджер
        if (TickManager)
            {
            TickManager->Tick ( DeltaTime );
            }

            // Безопасный тик акторов (базовый тик)
        for (size_t i = 0; i < Actors.size (); ++i)
            {
            CEActor * Actor = Actors[ i ];
            if (Actor && !Actor->IsPendingKill ())
                {
                Actor->Tick ( DeltaTime );
                }
            }              
      
        }

    void CEWorld::Destroy ()
        {
        std::string safeName = GetName ();
        CE_DEBUG ( "CEWorld '{}' beginning destruction", safeName );
        CEObject::Destroy ();
        }

    void CEWorld::ProcessPendingSpawns ()
        {
        if (PendingActors.empty ()) return;

        CE_DEBUG ( "CEWorld: Spawning {} pending actors", PendingActors.size () );

        for (CEActor * Actor : PendingActors)
            {
            if (Actor)
                {
                    // Устанавливаем TickManager и World ссылку актору ДО BeginPlay
                Actor->SetTickManager ( TickManager );
                Actor->SetWorld ( this );  // Добавляем эту строку!

                Actors.push_back ( Actor );
                Actor->BeginPlay (); // В BeginPlay будет RegisterTickFunctions()
                std::string actorName = Actor->GetName ();
                CE_DEBUG ( "CEWorld: Actor '{}' spawned", actorName );
                }
            }
        PendingActors.clear ();
        }

    void CEWorld::ProcessPendingKills ()
        {
        if (PendingKillActors.empty ()) return;

        CE_DEBUG ( "CEWorld: Destroying {} pending actors", PendingKillActors.size () );

        for (CEActor * Actor : PendingKillActors)
            {
            if (!Actor) continue;

            std::string actorName = Actor->GetName ();
            auto it = std::find ( Actors.begin (), Actors.end (), Actor );
            if (it != Actors.end ())
                {
                Actors.erase ( it );
                Actor->Destroy (); // В Destroy будет UnregisterTickFunctions()
                delete Actor;
                CE_DEBUG ( "CEWorld: Actor '{}' destroyed", actorName );
                }
            }
        PendingKillActors.clear ();
        }
    }