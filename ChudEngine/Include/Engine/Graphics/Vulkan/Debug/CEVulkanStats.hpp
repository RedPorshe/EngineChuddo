// Graphics/Vulkan/Debug/CEVulkanStats.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace CE
    {
    class CEVulkanContext;

    struct FrameStats
        {
        uint64_t frameNumber = 0;
        double frameTime = 0.0;
        double cpuTime = 0.0;
        double gpuTime = 0.0;
        uint32_t drawCalls = 0;
        uint32_t triangleCount = 0;
        uint32_t vertexCount = 0;
        size_t memoryUsed = 0;
        size_t memoryAllocated = 0;

        void Reset ();
        void Accumulate ( const FrameStats & other );
        };

    class CEVulkanStats
        {
        public:
            CEVulkanStats ( CEVulkanContext * context );
            ~CEVulkanStats ();

            bool Initialize ();
            void Shutdown ();

            void BeginFrame ();
            void EndFrame ();
            void BeginGPUQuery ( const std::string & name );
            void EndGPUQuery ( const std::string & name );

            void AddDrawCall ( uint32_t triangleCount, uint32_t vertexCount );
            void AddMemoryUsage ( size_t allocated, size_t used );

            const FrameStats & GetLastFrameStats () const { return m_LastFrameStats; }
            const FrameStats & GetAverageStats () const { return m_AverageStats; }
            double GetGPUTime ( const std::string & queryName ) const;

            void PrintFrameStats () const;
            std::string GetStatsString () const;

        private:
            void UpdateAverageStats ();
            bool CreateQueryPools ();

            CEVulkanContext * m_Context = nullptr;
            FrameStats m_CurrentFrameStats;
            FrameStats m_LastFrameStats;
            FrameStats m_AverageStats;

            VkQueryPool m_TimestampQueryPool = VK_NULL_HANDLE;
            VkQueryPool m_PipelineQueryPool = VK_NULL_HANDLE;

            std::unordered_map<std::string, uint32_t> m_QueryIndices;
            std::unordered_map<std::string, double> m_QueryResults;

            uint32_t m_FrameCount = 0;
            bool m_StatsEnabled = false;
        };
    }