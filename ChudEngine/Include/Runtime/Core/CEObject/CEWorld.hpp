// Source/Runtime/Core/CEObject/CEWorld.hpp
#pragma once

#include "CEObject.hpp"
#include "CEActor.hpp"
#include "CETickManager.hpp"  // Добавляем include
#include <vector>
#include <unordered_map>
#include <memory>

namespace CE
    {
    class CEActor;

    class CEWorld : public CEObject
        {
        public:
            CEWorld ( const std::string & WorldName = "MainWorld" );
            virtual ~CEWorld ();

            // Управление акторами
            void SpawnActor ( CEActor * Actor );
            void DestroyActor ( CEActor * Actor );
           
            template<typename T>
            std::vector<T *> GetComponentsOfType ()
                {
                std::vector<T *> components;
                for (auto * actor : Actors)
                    {
                    auto actorComponents = actor->GetComponentsOfType<T> ();
                    components.insert ( components.end (), actorComponents.begin (), actorComponents.end () );
                    }
                return components;
                }

            // Поиск акторов
            template<typename T>
            T * FindActorByName ( const std::string & Name )
                {
                for (CEActor * Actor : Actors)
                    {
                    if (Actor->GetName () == Name)
                        {
                        return Actor->CastTo<T>();
                        }
                    }
                return nullptr;
                }

            

            template<typename T>
            std::vector<T *> FindActorsOfType ()
                {
                std::vector<T *> result;
                for (CEActor * Actor : Actors)
                    {
                    if (T * CastActor = Actor->CastTo<T> ())
                        {
                        result.push_back ( CastActor );
                        }
                    }
                return result;
                }

                // Tick система
            CETickManager * GetTickManager () const { return TickManager; }

            // Статистика
            size_t GetActorCount () const { return Actors.size (); }
            size_t GetPendingSpawnCount () const { return PendingActors.size (); }

            // Переопределенные методы
            virtual void BeginPlay () override;
            virtual void Tick ( float DeltaTime ) override;
            virtual void Destroy () override;
            const std::vector<CEActor *> & GetActors () const { return Actors; }
        private:
            std::vector<CEActor *> Actors;
            std::vector<CEActor *> PendingActors;
            std::vector<CEActor *> PendingKillActors;
            CETickManager * TickManager;  // Добавляем TickManager

            void ProcessPendingSpawns ();
            void ProcessPendingKills ();
        };
    }