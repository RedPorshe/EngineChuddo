#pragma once

#include "Core/CEObject/CEObject.hpp"
#include <vector>
#include <functional>
#include <unordered_map>

namespace CE
    {
    enum class CETickGroup
        {
        PrePhysics,    // ����, ��������, ������
        Physics,       // ������, ��������
        PostPhysics,   // �������� ����� ������
        Effects,       // ���������� �������, ��������
        MAX
        };

    class CETickFunction
        {
        public:
            CETickFunction ( std::function<void ( float )> InFunction, CETickGroup InGroup, float InInterval = 0.0f );

            void Execute ( float DeltaTime );
            bool CanTick ( float DeltaTime ); // ������ const!

            CETickGroup GetTickGroup () const { return TickGroup; }
            float GetTickInterval () const { return TickInterval; }
            void SetTickInterval ( float Interval ) { TickInterval = Interval; }

            bool IsEnabled () const { return bEnabled; }
            void SetEnabled ( bool bEnable ) { bEnabled = bEnable; }

        private:
            std::function<void ( float )> TickFunction;
            CETickGroup TickGroup;
            float TickInterval;
            float AccumulatedTime;
            bool bEnabled;
        };

    class CETickManager : public CEObject
        {
        public:
            CETickManager ( const std::string & ManagerName = "CETickManager" );
            virtual ~CETickManager ();

            // ����������� ���-�������
            void RegisterTickFunction ( CEObject * Owner, std::function<void ( float )> Function,
                                        CETickGroup Group, float Interval = 0.0f );
            void UnregisterTickFunctions ( CEObject * Owner );

            // ���������� ������
            void SetTickGroupEnabled ( CETickGroup Group, bool bEnabled );
            void SetWorldTimeDilation ( float Dilation ) { TimeDilation = Dilation; }

            // �������� ����� ����������
           virtual void Tick ( float DeltaTime ) override;

        private:
            std::unordered_map<CEObject *, std::vector<CETickFunction>> OwnerTickFunctions;
            std::vector<CETickFunction> StandaloneTickFunctions;
            bool bTickGroupsEnabled[ static_cast< int >( CETickGroup::MAX ) ];
            float TimeDilation;

            void ProcessTickGroup ( CETickGroup Group, float DeltaTime );
        };
    }