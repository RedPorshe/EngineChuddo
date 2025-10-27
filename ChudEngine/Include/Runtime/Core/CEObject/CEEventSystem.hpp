#pragma once

#include "Core/CEObject/CEObject.hpp"
#include "Core/CEObject/CEDelegate.hpp"
#include "Core/CEObject/CEEvent.hpp" 
#include <unordered_map>
#include <string>
#include <memory>

namespace CE
    {
        // Система управления событиями
    class CEEventSystem : public CEObject
        {
        public:
            CEEventSystem ();
            virtual ~CEEventSystem ();

            // Регистрация обработчиков событий
            template<typename EventType>
            void RegisterHandler ( const std::string & EventName, std::function<void ( std::shared_ptr<EventType> )> Handler )
                {
                auto delegate = GetOrCreateDelegate<EventType> ( EventName );
                delegate->Bind ( Handler );
                CE_DEBUG ( "EventSystem: Registered handler for event '{}'", EventName );
                }

            template<typename EventType, typename ClassType>
            void RegisterHandler ( const std::string & EventName, ClassType * Object, void ( ClassType:: * Method )( std::shared_ptr<EventType> ) )
                {
                auto delegate = GetOrCreateDelegate<EventType> ( EventName );
                delegate->Bind ( Object, Method );
                CE_DEBUG ( "EventSystem: Registered object handler for event '{}'", EventName );
                }

                // Отправка событий
            template<typename EventType>
            void BroadcastEvent ( const std::string & EventName, std::shared_ptr<EventType> Event )
                {
                auto it = EventDelegates.find ( EventName );
                if (it != EventDelegates.end ())
                    {
                    if (auto delegate = std::dynamic_pointer_cast< CEDelegate<std::shared_ptr<EventType>> >( it->second ))
                        {
                        delegate->Broadcast ( Event );
                        CE_DEBUG ( "EventSystem: Broadcast event '{}' to {} handlers",
                                  EventName, delegate->GetNumBindings () );
                        }
                    }
                else
                    {
                    CE_WARN ( "EventSystem: No handlers registered for event '{}'", EventName );
                    }
                }

                // Удаление обработчиков
            void UnregisterAllHandlers ( const std::string & EventName );
            void Clear ();

        private:
            std::unordered_map<std::string, std::shared_ptr<CEDelegateBase>> EventDelegates;

            template<typename EventType>
            std::shared_ptr<CEDelegate<std::shared_ptr<EventType>>> GetOrCreateDelegate ( const std::string & EventName )
                {
                auto it = EventDelegates.find ( EventName );
                if (it == EventDelegates.end ())
                    {
                    auto delegate = std::make_shared<CEDelegate<std::shared_ptr<EventType>>> ();
                    delegate->SetName ( "Delegate_" + EventName );
                    EventDelegates[ EventName ] = delegate;
                    return delegate;
                    }
                return std::dynamic_pointer_cast< CEDelegate<std::shared_ptr<EventType>> >( it->second );
                }
        };
    }