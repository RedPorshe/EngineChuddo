#include "Graphics/Vulkan/Debug/CEVulkanStats.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace CE
    {
    void FrameStats::Reset ()
        {
        frameTime = 0.0;
        cpuTime = 0.0;
        gpuTime = 0.0;
        drawCalls = 0;
        triangleCount = 0;
        vertexCount = 0;
        memoryUsed = 0;
        memoryAllocated = 0;
        }

    void FrameStats::Accumulate ( const FrameStats & other )
        {
        frameTime += other.frameTime;
        cpuTime += other.cpuTime;
        gpuTime += other.gpuTime;
        drawCalls += other.drawCalls;
        triangleCount += other.triangleCount;
        vertexCount += other.vertexCount;
        memoryUsed += other.memoryUsed;
        memoryAllocated += other.memoryAllocated;
        }

    CEVulkanStats::CEVulkanStats ( CEVulkanContext * context )
        : m_Context ( context )
        , m_FrameCount ( 0 )
        , m_StatsEnabled ( false )
        {
        m_CurrentFrameStats.Reset ();
        m_LastFrameStats.Reset ();
        m_AverageStats.Reset ();
        }

    CEVulkanStats::~CEVulkanStats ()
        {
        Shutdown ();
        }

    bool CEVulkanStats::Initialize ()
        {
        if (m_StatsEnabled)
            {
            return true;
            }

        try
            {
            if (!CreateQueryPools ())
                {
                throw std::runtime_error ( "Failed to create query pools" );
                }

            m_StatsEnabled = true;
            CE_CORE_DEBUG ( "Vulkan stats initialized" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan stats: {}", e.what () );
                return false;
                }
        }

    void CEVulkanStats::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            if (m_TimestampQueryPool != VK_NULL_HANDLE)
                {
                vkDestroyQueryPool ( device, m_TimestampQueryPool, nullptr );
                m_TimestampQueryPool = VK_NULL_HANDLE;
                }

            if (m_PipelineQueryPool != VK_NULL_HANDLE)
                {
                vkDestroyQueryPool ( device, m_PipelineQueryPool, nullptr );
                m_PipelineQueryPool = VK_NULL_HANDLE;
                }
            }

        m_StatsEnabled = false;
        CE_CORE_DEBUG ( "Vulkan stats shut down" );
        }

    void CEVulkanStats::BeginFrame ()
        {
        if (!m_StatsEnabled) return;

        m_CurrentFrameStats.Reset ();
        m_CurrentFrameStats.frameNumber = m_FrameCount;

        // Reset query pools for new frame
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            if (m_TimestampQueryPool != VK_NULL_HANDLE)
                {
                vkResetQueryPool ( device, m_TimestampQueryPool, 0, 2 ); // Start and end timestamps
                }

            if (m_PipelineQueryPool != VK_NULL_HANDLE)
                {
                vkResetQueryPool ( device, m_PipelineQueryPool, 0, 1 ); // Pipeline statistics
                }

                // Write start timestamp
            vkCmdWriteTimestamp ( m_Context->GetDevice ()->GetGraphicsQueue (),
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  m_TimestampQueryPool, 0 );
            }
        }

    void CEVulkanStats::EndFrame ()
        {
        if (!m_StatsEnabled) return;

        // Write end timestamp
        if (m_Context && m_Context->GetDevice ())
            {
            vkCmdWriteTimestamp ( m_Context->GetDevice ()->GetGraphicsQueue (),
                                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                  m_TimestampQueryPool, 1 );

               // Get timestamp results
            uint64_t timestamps[ 2 ];
            VkResult result = vkGetQueryPoolResults (
                m_Context->GetDevice ()->GetDevice (),
                m_TimestampQueryPool,
                0, 2,
                sizeof ( timestamps ),
                timestamps,
                sizeof ( uint64_t ),
                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
            );

            if (result == VK_SUCCESS)
                {
// Calculate GPU time (convert timestamp ticks to milliseconds)
                float timestampPeriod = m_Context->GetDevice ()->GetPhysicalDeviceProperties ().limits.timestampPeriod;
                m_CurrentFrameStats.gpuTime = ( timestamps[ 1 ] - timestamps[ 0 ] ) * timestampPeriod * 1e-6f; // to milliseconds
                }
            }

            // Update last frame stats
        m_LastFrameStats = m_CurrentFrameStats;

        // Update average stats
        UpdateAverageStats ();

        m_FrameCount++;
        }

    void CEVulkanStats::BeginGPUQuery ( const std::string & name )
        {
        if (!m_StatsEnabled) return;

        // In a full implementation, this would manage multiple named queries
        CE_CORE_TRACE ( "Beginning GPU query: {}", name );
        }

    void CEVulkanStats::EndGPUQuery ( const std::string & name )
        {
        if (!m_StatsEnabled) return;

        CE_CORE_TRACE ( "Ending GPU query: {}", name );
        }

    void CEVulkanStats::AddDrawCall ( uint32_t triangleCount, uint32_t vertexCount )
        {
        m_CurrentFrameStats.drawCalls++;
        m_CurrentFrameStats.triangleCount += triangleCount;
        m_CurrentFrameStats.vertexCount += vertexCount;
        }

    void CEVulkanStats::AddMemoryUsage ( size_t allocated, size_t used )
        {
        m_CurrentFrameStats.memoryAllocated += allocated;
        m_CurrentFrameStats.memoryUsed += used;
        }

    double CEVulkanStats::GetGPUTime ( const std::string & queryName ) const
        {
        auto it = m_QueryResults.find ( queryName );
        if (it != m_QueryResults.end ())
            {
            return it->second;
            }
        return 0.0;
        }

    void CEVulkanStats::PrintFrameStats () const
        {
        std::string stats = GetStatsString ();
        CE_CORE_INFO ( "Frame Stats:\n{}", stats );
        }

    std::string CEVulkanStats::GetStatsString () const
        {
        std::stringstream ss;

        ss << std::fixed << std::setprecision ( 2 );
        ss << "Frame: " << m_LastFrameStats.frameNumber << "\n";
        ss << "CPU Time: " << m_LastFrameStats.cpuTime << " ms\n";
        ss << "GPU Time: " << m_LastFrameStats.gpuTime << " ms\n";
        ss << "Draw Calls: " << m_LastFrameStats.drawCalls << "\n";
        ss << "Triangles: " << m_LastFrameStats.triangleCount << "\n";
        ss << "Vertices: " << m_LastFrameStats.vertexCount << "\n";
        ss << "Memory Used: " << ( m_LastFrameStats.memoryUsed / ( 1024.0 * 1024.0 ) ) << " MB\n";
        ss << "Memory Allocated: " << ( m_LastFrameStats.memoryAllocated / ( 1024.0 * 1024.0 ) ) << " MB\n";

        if (m_FrameCount > 0)
            {
            ss << "--- Averages (Last " << m_FrameCount << " frames) ---\n";
            ss << "Avg CPU Time: " << ( m_AverageStats.cpuTime / m_FrameCount ) << " ms\n";
            ss << "Avg GPU Time: " << ( m_AverageStats.gpuTime / m_FrameCount ) << " ms\n";
            ss << "Avg Draw Calls: " << ( m_AverageStats.drawCalls / m_FrameCount ) << "\n";
            }

        return ss.str ();
        }

    void CEVulkanStats::UpdateAverageStats ()
        {
            // Simple moving average
        if (m_FrameCount == 0)
            {
            m_AverageStats = m_LastFrameStats;
            }
        else
            {
                 // Exponential moving average for smoother results
            const float alpha = 0.1f; // Smoothing factor
            m_AverageStats.frameTime = m_AverageStats.frameTime * ( 1 - alpha ) + m_LastFrameStats.frameTime * alpha;
            m_AverageStats.cpuTime = m_AverageStats.cpuTime * ( 1 - alpha ) + m_LastFrameStats.cpuTime * alpha;
            m_AverageStats.gpuTime = m_AverageStats.gpuTime * ( 1 - alpha ) + m_LastFrameStats.gpuTime * alpha;
            m_AverageStats.drawCalls = static_cast< uint32_t >( m_AverageStats.drawCalls * ( 1 - alpha ) + m_LastFrameStats.drawCalls * alpha );
            m_AverageStats.triangleCount = static_cast< uint32_t >( m_AverageStats.triangleCount * ( 1 - alpha ) + m_LastFrameStats.triangleCount * alpha );
            m_AverageStats.vertexCount = static_cast< uint32_t >( m_AverageStats.vertexCount * ( 1 - alpha ) + m_LastFrameStats.vertexCount * alpha );
            }
        }

    bool CEVulkanStats::CreateQueryPools ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            return false;
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();

        // Create timestamp query pool
        VkQueryPoolCreateInfo timestampQueryPoolInfo = {};
        timestampQueryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        timestampQueryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        timestampQueryPoolInfo.queryCount = 2; // Start and end timestamps

        VkResult result = vkCreateQueryPool ( device, &timestampQueryPoolInfo, nullptr, &m_TimestampQueryPool );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create timestamp query pool" );
            return false;
            }

            // Check if pipeline statistics are supported
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties ( m_Context->GetDevice ()->GetPhysicalDevice (), &properties );

        if (properties.limits.pipelineStatisticsQuery)
            {
// Create pipeline statistics query pool
            VkQueryPoolCreateInfo pipelineQueryPoolInfo = {};
            pipelineQueryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            pipelineQueryPoolInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
            pipelineQueryPoolInfo.queryCount = 1;
            pipelineQueryPoolInfo.pipelineStatistics =
                VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
                VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;

            result = vkCreateQueryPool ( device, &pipelineQueryPoolInfo, nullptr, &m_PipelineQueryPool );
            if (result != VK_SUCCESS)
                {
                CE_CORE_WARN ( "Pipeline statistics not supported, continuing without them" );
                }
            }
        else
            {
            CE_CORE_WARN ( "Pipeline statistics not supported on this device" );
            }

        return true;
        }
    }