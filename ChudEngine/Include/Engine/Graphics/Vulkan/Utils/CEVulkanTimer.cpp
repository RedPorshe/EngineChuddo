#include "Graphics/Vulkan/Utils/CEVulkanTimer.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanTimer::CEVulkanTimer ()
        {
        }

    void CEVulkanTimer::Start ( const std::string & name )
        {
        auto & timer = m_Timers[ name ];
        timer.startTime = std::chrono::high_resolution_clock::now ();
        timer.running = true;
        }

    void CEVulkanTimer::Stop ( const std::string & name )
        {
        auto it = m_Timers.find ( name );
        if (it != m_Timers.end () && it->second.running)
            {
            auto endTime = std::chrono::high_resolution_clock::now ();
            auto duration = std::chrono::duration_cast< std::chrono::microseconds >( endTime - it->second.startTime );
            it->second.accumulatedTime += duration.count () / 1000.0; // Convert to milliseconds
            it->second.running = false;
            }
        }

    double CEVulkanTimer::GetElapsedTime ( const std::string & name ) const
        {
        auto it = m_Timers.find ( name );
        if (it != m_Timers.end ())
            {
            return it->second.accumulatedTime;
            }
        return 0.0;
        }

    void CEVulkanTimer::Reset ( const std::string & name )
        {
        auto it = m_Timers.find ( name );
        if (it != m_Timers.end ())
            {
            it->second.accumulatedTime = 0.0;
            it->second.running = false;
            }
        }

    void CEVulkanTimer::ResetAll ()
        {
        for (auto & [name, timer] : m_Timers)
            {
            timer.accumulatedTime = 0.0;
            timer.running = false;
            }
        }

    void CEVulkanTimer::PrintTimings () const
        {
        CE_CORE_INFO ( "=== Timer Results ===" );
        for (const auto & [name, timer] : m_Timers)
            {
            CE_CORE_INFO ( "  {}: {:.3f} ms", name, timer.accumulatedTime );
            }
        }
    }