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

            // Фабричный метод для создания объектов этого класса
            template<typename T>
            T * CreateObject ()
                {
                static_assert( std::is_base_of<CEObject, T>::value, "T must inherit from UObject" );
                return new T ();
                }

                // Проверка наследования
            bool IsChildOf ( const CEClass * OtherClass ) const;

        private:
            std::string ClassName;
            CEClass * SuperClass;
        };

        // Макрос для объявления класса (упрощенная версия UE)
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

    // Макрос для реализации класса
#define IMPLEMENT_CLASS(ClassName)
    }