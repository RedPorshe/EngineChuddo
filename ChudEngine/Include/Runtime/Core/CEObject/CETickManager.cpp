#include "Core/CEObject/CETickManager.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CETickFunction::CETickFunction ( std::function<void ( float )> InFunction, CETickGroup InGroup, float InInterval )
        : TickFunction ( InFunction ), TickGroup ( InGroup ), TickInterval ( InInterval ),
        AccumulatedTime ( 0.0f ), bEnabled ( true )
        {
        }

    void CETickFunction::Execute ( float DeltaTime )
        {
        if (bEnabled && TickFunction)
            {
            TickFunction ( DeltaTime );
            }
        }

    bool CETickFunction::CanTick ( float DeltaTime )  // Убрали const!
        {
        if (!bEnabled) return false;

        if (TickInterval > 0.0f)
            {
            AccumulatedTime += DeltaTime;
            if (AccumulatedTime >= TickInterval)
                {
                AccumulatedTime = 0.0f;
                return true;
                }
            return false;
            }

        return true;
        }

    CETickManager::CETickManager ( const std::string & ManagerName )
        : TimeDilation ( 1.0f ) , CEObject ( ManagerName )
        {
        SetName ( "CETickManager" );

        // Все группы тиков включены по умолчанию
        for (int i = 0; i < static_cast< int > ( CETickGroup::MAX ); ++i)
            {
            bTickGroupsEnabled[ i ] = true;
            }

        CE_DEBUG ( "CETickManager created" );
        }

    CETickManager::~CETickManager ()
        {
        CE_DEBUG ( "CETickManager destroyed" );
        }

    void CETickManager::RegisterTickFunction ( CEObject * Owner, std::function<void ( float )> Function,
                                               CETickGroup Group, float Interval )
        {
        if (!Owner || !Function) return;

        CETickFunction tickFunc ( Function, Group, Interval );
        OwnerTickFunctions[ Owner ].push_back ( tickFunc );

        std::string safeName = Owner->GetName ();
        CE_DEBUG ( "Registered tick function for '{}' in group {}", safeName, static_cast< int >( Group ) );
        }

    void CETickManager::UnregisterTickFunctions ( CEObject * Owner )
        {
        if (!Owner) return;

        auto it = OwnerTickFunctions.find ( Owner );
        if (it != OwnerTickFunctions.end ())
            {
            OwnerTickFunctions.erase ( it );
            std::string safeName = Owner->GetName ();
            CE_DEBUG ( "Unregistered all tick functions for '{}'", safeName );
            }
        }

    void CETickManager::SetTickGroupEnabled ( CETickGroup Group, bool bEnabled )
        {
        int groupIndex = static_cast< int >( Group );
        if (groupIndex >= 0 && groupIndex < static_cast< int > ( CETickGroup::MAX ))
            {
            bTickGroupsEnabled[ groupIndex ] = bEnabled;
            CE_DEBUG ( "Tick group {} {}", static_cast< int > ( Group ), bEnabled ? "enabled" : "disabled" );
            }
        }

    void CETickManager::Tick ( float DeltaTime )
        {
        float dilatedDelta = DeltaTime * TimeDilation;

        // Обновляем в порядке приоритета
        for (int i = 0; i < static_cast< int > ( CETickGroup::MAX ); ++i)
            {
            if (bTickGroupsEnabled[ i ])
                {
                ProcessTickGroup ( static_cast< CETickGroup > ( i ), dilatedDelta );
                }
            }

       // CE_DEBUG ( "CETickManager processed {} tick groups", static_cast< int > ( CETickGroup::MAX ) );
        }

    void CETickManager::ProcessTickGroup ( CETickGroup Group, float DeltaTime )
        {
            // Обрабатываем тик-функции владельцев
        for (auto & ownerPair : OwnerTickFunctions)
            {
            for (auto & tickFunc : ownerPair.second)
                {
                if (tickFunc.GetTickGroup () == Group && tickFunc.CanTick ( DeltaTime ))
                    {
                    tickFunc.Execute ( DeltaTime );
                    }
                }
            }

            // Обрабатываем standalone тик-функции
        for (auto & tickFunc : StandaloneTickFunctions)
            {
            if (tickFunc.GetTickGroup () == Group && tickFunc.CanTick ( DeltaTime ))
                {
                tickFunc.Execute ( DeltaTime );
                }
            }
        }
    }