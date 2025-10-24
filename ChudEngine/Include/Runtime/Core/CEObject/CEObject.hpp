#pragma once
#include "Utils/Logger.hpp"
#include "Core/CoreTypes.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <random>
#include <mutex>

namespace CE
    {
    class CEClass;

    class CEObject
        {
        public:
            CEObject ();
            explicit CEObject ( const std::string & ObjectName ); // Новый конструктор с именем
            virtual ~CEObject ();

            // Базовые функции
            virtual void BeginPlay ();
            virtual void Tick ( float DeltaTime );
            virtual void Destroy ();

            // Система имен
            const std::string & GetName () const { return Name; }
            void SetName ( const std::string & NewName ) { Name = NewName; }

            // Уникальный идентификатор
            uint64 GetUniqueID () const { return UniqueID; }

            // Флаги объекта
            bool IsPendingKill () const { return bPendingKill; }
            bool IsInitialized () const { return bInitialized; }

            // Получение класса объекта
            virtual CEClass * GetClass () const { return nullptr; }



            // Приведение типов
            template<typename T>
            T * CastTo ()
                {
                return dynamic_cast< T * >( this );
                }

            template<typename T>
            const T * CastTo () const
                {
                return dynamic_cast< const T * >( this );
                }

            // Статические методы
            static CEObject * FindObjectByName ( const std::string & Name );
            static CEObject * FindObjectByID ( uint64 ID );
            static std::vector<CEObject *> FindObjectsOfType ( CEClass * Class );
            static uint64 GenerateID ();

        protected:
            std::string Name;
            uint64 UniqueID;
            bool bPendingKill;
            bool bInitialized;

        private:
            static std::unordered_map<uint64, CEObject *> AllObjects;
            static std::mutex IDMutex;

            // Генератор случайных чисел (объявление)
            static std::random_device RandomDevice;
            static std::mt19937_64 RandomGenerator;
            static std::uniform_int_distribution<uint64> RandomDistribution;
        };
    }