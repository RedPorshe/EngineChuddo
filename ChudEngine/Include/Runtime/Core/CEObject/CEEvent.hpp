// Source/Runtime/Core/CEObject/CEEvent.hpp
#pragma once

#include "CEObject.hpp"

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