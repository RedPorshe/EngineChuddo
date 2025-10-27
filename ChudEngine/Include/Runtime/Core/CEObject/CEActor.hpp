#pragma once

#include "Utils/Logger.hpp"
#include "Core/CEObject/CEObject.hpp"
#include "Core/CEObject/Components/CEComponent.hpp"
#include "Core/CEObject/Components/CETransformComponent.hpp"
#include "Core/Containers/CEArray.hpp"
#include <algorithm>
#include <memory>
#include <vector>
#include <typeindex>
#include <unordered_map>


namespace CE
    {
    class CETickManager;
    class CEWorld;

    class CEActor : public CEObject
        {
        public:
            CEActor ( const std::string & ActorName = "CEActor" );
            virtual ~CEActor () override;

            CETransformComponent * GetTransform () const { return TransformComponent; }

            // Component system
            template<typename T>
            T * AddComponent ();

            template<typename T>
            T * GetComponent ();

            template<typename T>
            CEArray<T *> GetComponents ();

            // Lifecycle
            virtual void BeginPlay () override;
            virtual void Tick ( float DeltaTime ) override;

            template<typename T, typename... Args>
            T * CreateComponent ( Args&&... args )
                {
                auto component = std::make_unique<T> ( std::forward<Args> ( args )... );
                T * rawPtr = component.get ();

                // Устанавливаем владельца и регистрируем компонент
                component->SetOwner ( this );
                Components.push_back ( std::move ( component ) );
                RegisterComponent ( rawPtr ); // ВАЖНО: регистрируем компонент

                return rawPtr;
                }

                // Метод для получения компонентов по типу
            template<typename T>
            std::vector<T *> GetComponentsOfType ()
                {
                std::vector<T *> components;
                for (auto & component : Components)
                    {
                    if (auto * casted = dynamic_cast< T * >( component.get () ))
                        {
                        components.push_back ( casted );
                        }
                    }
                return components;
                }

                // World and Tick management
            void SetTickManager ( CETickManager * InTickManager ) { TickManager = InTickManager; }
            void SetWorld ( CEWorld * InWorld ) { World = InWorld; }

        protected:
            std::vector<std::unique_ptr<CEComponent>> Components;
            std::unordered_map<std::type_index, CEArray<CEComponent *>> ComponentMap;
            CETransformComponent * TransformComponent = nullptr;
        private:
            CETickManager * TickManager = nullptr;
            CEWorld * World = nullptr;

            void RegisterComponent ( CEComponent * Component );
            bool IsInGame () const { return bInitialized && !bPendingKill; }

            // Добавляем флаги состояния
            bool bInitialized = false;
            bool bPendingKill = false;
        };

        // Реализации шаблонных методов остаются без изменений
    template<typename T>
    T * CEActor::AddComponent ()
        {
        static_assert( std::is_base_of_v<CEComponent, T>, "T must inherit from CEComponent" );

        auto NewComponent = std::make_unique<T> ();
        T * ComponentPtr = NewComponent.get ();
        NewComponent->SetOwner ( this );

        Components.push_back ( std::move ( NewComponent ) );
        RegisterComponent ( ComponentPtr );

        if (IsInGame ())
            {
            ComponentPtr->BeginPlay ();
            }
        return ComponentPtr;
        }

    template<typename T>
    T * CEActor::GetComponent ()
        {
        static_assert( std::is_base_of_v<CEComponent, T>, "T must inherit from CEComponent" );

        auto it = ComponentMap.find ( typeid( T ) );
        if (it != ComponentMap.end () && !it->second.IsEmpty ())
            {
            return static_cast< T * >( it->second[ 0 ] );
            }
        return nullptr;
        }

    template<typename T>
    CEArray<T *> CEActor::GetComponents ()
        {
        static_assert( std::is_base_of_v<CEComponent, T>, "T must inherit from CEComponent" );

        CEArray<T *> Result;
        auto it = ComponentMap.find ( typeid( T ) );
        if (it != ComponentMap.end ())
            {
            for (CEComponent * Comp : it->second)
                {
                Result.PushBack ( static_cast< T * >( Comp ) );
                }
            }
        return Result;
        }
    }