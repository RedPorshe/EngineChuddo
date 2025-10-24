// Source/Framework/Utils/UUID.h
#pragma once

#include <cstdint>
#include <random>

namespace CE
    {
    class UUID
        {
        public:
            UUID ();
            UUID ( uint64_t uuid );
            UUID ( const UUID & other );

            operator uint64_t() const { return m_UUID; }

        private:
            uint64_t m_UUID;

            // Статические переменные должны быть объявлены здесь
            static std::random_device s_RandomDevice;
            static std::mt19937_64 s_Engine;
            static std::uniform_int_distribution<uint64_t> s_UniformDistribution;
        };
    }