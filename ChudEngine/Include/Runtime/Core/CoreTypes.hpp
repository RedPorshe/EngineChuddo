// ChudEngine/Include/Runtime/Core/CoreTypes.h
#pragma once

#include <cstdint>
#include <string>

namespace CE
    {
        // Базовые целочисленные типы
    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;

    using byte = uint8_t;

    // Вещественные типы
    using float32 = float;
    using float64 = double;

    // Строковые типы
    using FString = std::string;
    using FName = std::string;

    // Логический тип
    using Bool = bool;

    // Специальные флаги
    static constexpr uint64 INDEX_NONE = -1;


    }

#ifndef CE_CORE_ASSERT
#ifdef _DEBUG
#include <cassert>
#define CE_CORE_ASSERT(condition, message) assert(condition && message)
#else
#define CE_CORE_ASSERT(condition, message) ((void)0)
#endif
#endif
#include "Utils/Logger.hpp"