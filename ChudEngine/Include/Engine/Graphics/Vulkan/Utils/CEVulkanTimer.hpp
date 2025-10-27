// Graphics/Vulkan/Utils/CEVulkanTimer.hpp
#pragma once
#include <chrono>
#include <string>
#include <unordered_map>

namespace CE
    {
    class CEVulkanTimer
        {
        public:
            CEVulkanTimer ();
            ~CEVulkanTimer () = default;

            void Start ( const std::string & name );
            void Stop ( const std::string & name );
            double GetElapsedTime ( const std::string & name ) const;
            void Reset ( const std::string & name );
            void ResetAll ();

            void PrintTimings () const;

        private:
            struct TimerData
                {
                std::chrono::high_resolution_clock::time_point startTime;
                double accumulatedTime = 0.0;
                bool running = false;
                };

            std::unordered_map<std::string, TimerData> m_Timers;
        };
    }