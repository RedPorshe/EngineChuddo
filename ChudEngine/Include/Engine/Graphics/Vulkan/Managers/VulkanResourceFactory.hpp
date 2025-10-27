#pragma once
#include "Graphics/Vulkan/Core/CEVulkanBuffer.hpp"
#include "Graphics/Vulkan/Core/VulkanDevice.hpp"
#include <memory>

namespace CE
    {
    class VulkanResourceFactory
        {
        public:
            static std::unique_ptr<CEVulkanBuffer> CreateVertexBuffer (
                VulkanDevice * device,
                VkDeviceSize size,
                const void * data = nullptr );

            static std::unique_ptr<CEVulkanBuffer> CreateIndexBuffer (
                VulkanDevice * device,
                VkDeviceSize size,
                const void * data = nullptr );

            static std::unique_ptr<CEVulkanBuffer> CreateUniformBuffer (
                VulkanDevice * device,
                VkDeviceSize size );
        };
    }