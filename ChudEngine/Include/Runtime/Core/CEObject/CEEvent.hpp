#pragma once
#include "Core/CEObject/CEObject.hpp"

namespace CE
    {
        // Базовый класс для всех событий
    class CEEvent : public CEObject
        {
        public:
            CEEvent () = default;
            virtual ~CEEvent () = default;
        };
    }