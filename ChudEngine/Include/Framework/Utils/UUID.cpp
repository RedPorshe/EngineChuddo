// Source/Framework/Utils/UUID.cpp
#include "Utils/UUID.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
        // Инициализация статических переменных
    std::random_device UUID::s_RandomDevice;
    std::mt19937_64 UUID::s_Engine ( s_RandomDevice () );
    std::uniform_int_distribution<uint64_t> UUID::s_UniformDistribution ( 1, UINT64_MAX );

    UUID::UUID ()
        : m_UUID ( s_UniformDistribution ( s_Engine ) )
        {
        CE_DEBUG ( "Generated UUID: {0}", m_UUID );
        }

    UUID::UUID ( uint64_t uuid )
        : m_UUID ( uuid )
        {
        }

    UUID::UUID ( const UUID & other )
        : m_UUID ( other.m_UUID )
        {
        }
    }