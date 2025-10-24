#include "Core/CEObject/CEObject.hpp"
#include <algorithm>

namespace CE
    {
    // Инициализация статических членов
    std::unordered_map<uint64, CEObject *> CEObject::AllObjects;
    std::mutex CEObject::IDMutex;

    std::random_device CEObject::RandomDevice;
    std::mt19937_64 CEObject::RandomGenerator ( CEObject::RandomDevice () );
    std::uniform_int_distribution<uint64> CEObject::RandomDistribution ( 1, UINT64_MAX - 1 );

    uint64 CEObject::GenerateID ()
        {
        std::lock_guard<std::mutex> lock ( IDMutex );

        uint64 newID;
        int attempts = 0;
        const int MAX_ATTEMPTS = 1000;

        do
            {
            newID = RandomDistribution ( RandomGenerator );
            attempts++;

            if (attempts >= MAX_ATTEMPTS)
                {
                CE_CORE_CRITICAL ( "Failed to generate unique ID after {} attempts", MAX_ATTEMPTS );
                throw std::runtime_error ( "CEObject ID generation failed" );
                }

            } while (AllObjects.find ( newID ) != AllObjects.end ());

        CE_DEBUG ( "Generated new object ID: {}", newID );
        return newID;
        }

    CEObject::CEObject ()
        : Name ( "CEObject" ), UniqueID ( GenerateID () ), bPendingKill ( false ), bInitialized ( false )
        {
        std::lock_guard<std::mutex> lock ( IDMutex );
        AllObjects[ UniqueID ] = this;

        std::string safeName = Name;
        CE_DEBUG ( "CEObject '{}' created (ID: {})", safeName, UniqueID );
        }

    CEObject::CEObject ( const std::string & ObjectName )
        : Name ( ObjectName ), UniqueID ( GenerateID () ), bPendingKill ( false ), bInitialized ( false )
        {
        std::lock_guard<std::mutex> lock ( IDMutex );
        AllObjects[ UniqueID ] = this;

        std::string safeName = Name;
        CE_DEBUG ( "CEObject '{}' created (ID: {})", safeName, UniqueID );
        }

    CEObject::~CEObject ()
        {
        std::string safeName = Name;
        uint64 safeID = UniqueID;

        {
        std::lock_guard<std::mutex> lock ( IDMutex );
        auto it = AllObjects.find ( safeID );
        if (it != AllObjects.end ())
            {
            AllObjects.erase ( it );
            }
        }

        CE_DEBUG ( "CEObject '{}' destroyed (ID: {})", safeName, safeID );
        }

    void CEObject::BeginPlay ()
        {
        bInitialized = true;
        std::string safeName = Name;
        CE_DEBUG ( "CEObject BeginPlay: {}", safeName );
        }

    void CEObject::Tick ( float DeltaTime )
        {
        // Базовая реализация пуста
        }

    void CEObject::Destroy ()
        {
        bPendingKill = true;
        std::string safeName = Name;
        CE_DEBUG ( "CEObject marked for destruction: {}", safeName );
        }

    CEObject * CEObject::FindObjectByName ( const std::string & Name )
        {
        std::lock_guard<std::mutex> lock ( IDMutex );

        for (auto & pair : AllObjects)
            {
            if (pair.second && pair.second->Name == Name)
                {
                return pair.second;
                }
            }
        return nullptr;
        }

    CEObject * CEObject::FindObjectByID ( uint64 ID )
        {
        std::lock_guard<std::mutex> lock ( IDMutex );

        auto it = AllObjects.find ( ID );
        if (it != AllObjects.end ())
            {
            return it->second;
            }
        return nullptr;
        }

    std::vector<CEObject *> CEObject::FindObjectsOfType ( CEClass * Class )
        {
        std::vector<CEObject *> Result;
        // TODO: Реализовать когда будет система классов
        return Result;
        }
    }