#pragma once

#include "Runtime/Core/CEObject/CEObject.hpp"
#include <string>
#include <vector>
#include <functional>

namespace CE
    {
    class CEClass : public CEObject
        {
        public:
            CEClass ( const std::string & ClassName, CEClass * SuperClass = nullptr );

            CEClass * GetSuperClass () const { return SuperClass; }
            const std::string & GetClassName () const { return ClassName; }

            // ��������� ����� ��� �������� �������� ����� ������
            template<typename T>
            T * CreateObject ()
                {
                static_assert( std::is_base_of<CEObject, T>::value, "T must inherit from UObject" );
                return new T ();
                }

                // �������� ������������
            bool IsChildOf ( const CEClass * OtherClass ) const;

        private:
            std::string ClassName;
            CEClass * SuperClass;
        };

        // ������ ��� ���������� ������ (���������� ������ UE)
#define DECLARE_CLASS(ClassName, SuperClassName) \
    public: \
        static CEClass* StaticClass() { \
            static CEClass* Class = nullptr; \
            if (!Class) { \
                Class = new CEClass(#ClassName, SuperClassName::StaticClass()); \
            } \
            return Class; \
        } \
        virtual CEClass* GetClass() const override { return StaticClass(); }

    // ������ ��� ���������� ������
#define IMPLEMENT_CLASS(ClassName)
    }