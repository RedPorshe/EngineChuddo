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
            explicit CEObject ( const std::string & ObjectName ); // ����� ����������� � ������
            virtual ~CEObject ();

            // ������� �������
            virtual void BeginPlay ();
            virtual void Tick ( float DeltaTime );
            virtual void Destroy ();

            // ������� ����
            const std::string & GetName () const { return Name; }
            void SetName ( const std::string & NewName ) { Name = NewName; }

            // ���������� �������������
            uint64 GetUniqueID () const { return UniqueID; }

            // ����� �������
            bool IsPendingKill () const { return bPendingKill; }
            bool IsInitialized () const { return bInitialized; }

            // ��������� ������ �������
            virtual CEClass * GetClass () const { return nullptr; }



            // ���������� �����
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

            // ����������� ������
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

            // ��������� ��������� ����� (����������)
            static std::random_device RandomDevice;
            static std::mt19937_64 RandomGenerator;
            static std::uniform_int_distribution<uint64> RandomDistribution;
        };
    }