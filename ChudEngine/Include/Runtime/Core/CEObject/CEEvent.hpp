#pragma once
#include "Core/CEObject/CEObject.hpp"

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