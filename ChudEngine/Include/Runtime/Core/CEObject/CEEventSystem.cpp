#include "Core/CEObject/CEEventSystem.hpp"

#include "Core/Logger.h"

namespace CE
    {
    CEEventSystem::CEEventSystem ()
        {
        SetName ( "CEEventSystem" );
        CE_DEBUG ( "CEEventSystem created" );
        }

    CEEventSystem::~CEEventSystem ()
        {
        Clear ();
        CE_DEBUG ( "CEEventSystem destroyed" );
        }

    void CEEventSystem::UnregisterAllHandlers ( const std::string & EventName )
        {
        auto it = EventDelegates.find ( EventName );
        if (it != EventDelegates.end ())
            {
            it->second->Clear ();
            EventDelegates.erase ( it );
            CE_DEBUG ( "EventSystem: Unregistered all handlers for event '{}'", EventName );
            }
        }

    void CEEventSystem::Clear ()
        {
        EventDelegates.clear ();
        CE_DEBUG ( "EventSystem: Cleared all event handlers" );
        }
    }