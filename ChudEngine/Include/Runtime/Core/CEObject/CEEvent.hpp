// Source/Runtime/Core/CEObject/CEEvent.hpp
#pragma once

#include "CEObject.hpp"

namespace CE
    {
        // ������� ����� ��� ���� �������
    class CEEvent : public CEObject
        {
        public:
            CEEvent () = default;
            virtual ~CEEvent () = default;
        };
    }