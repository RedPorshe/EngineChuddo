// Graphics/Vulkan/Managers/CEVulkanResourceManager.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <unordered_map>
#include <string>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanBuffer;
    class CEVulkanImage;

    class CEVulkanResourceManager
        {
        public:
            CEVulkanResourceManager ( CEVulkanContext * context );
            ~CEVulkanResourceManager ();

            // Buffer management
            std::shared_ptr<CEVulkanBuffer> CreateBuffer ( const std::string & name, VkDeviceSize size,
                                                           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties );
            std::shared_ptr<CEVulkanBuffer> GetBuffer ( const std::string & name );
            void DestroyBuffer ( const std::string & name );

            // Image management
            std::shared_ptr<CEVulkanImage> CreateImage ( const std::string & name, uint32_t width, uint32_t height,
                                                         VkFormat format, VkImageUsageFlags usage );
            std::shared_ptr<CEVulkanImage> GetImage ( const std::string & name );
            void DestroyImage ( const std::string & name );

            // Cleanup
            void Cleanup ();
            void PrintMemoryStatistics () const;

        private:
            CEVulkanContext * m_Context = nullptr;
            std::unordered_map<std::string, std::shared_ptr<CEVulkanBuffer>> m_Buffers;
            std::unordered_map<std::string, std::shared_ptr<CEVulkanImage>> m_Images;

            size_t m_TotalBufferMemory = 0;
            size_t m_TotalImageMemory = 0;
        };
    }