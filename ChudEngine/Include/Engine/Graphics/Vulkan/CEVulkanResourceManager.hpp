// Runtime/Renderer/Vulkan/CEVulkanResourceManager.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <unordered_map>
#include <string>
#include "CEVulkanBuffer.hpp"
#include "Core/Logger.h"

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanResourceManager
        {
        public:
            CEVulkanResourceManager ( CEVulkanContext * context );
            ~CEVulkanResourceManager ();

            // Управление буферами
            std::shared_ptr<CEVulkanBuffer> CreateBuffer (
                const std::string & name,
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties );

            std::shared_ptr<CEVulkanBuffer> GetBuffer ( const std::string & name );
            void DestroyBuffer ( const std::string & name );

            // Статистика
            void PrintMemoryStatistics () const;
            void Cleanup ();

        private:
            CEVulkanContext * m_Context = nullptr;
            std::unordered_map<std::string, std::shared_ptr<CEVulkanBuffer>> m_Buffers;

            // Статистика
            size_t m_TotalAllocatedMemory = 0;
            size_t m_BufferCount = 0;
        };
    }