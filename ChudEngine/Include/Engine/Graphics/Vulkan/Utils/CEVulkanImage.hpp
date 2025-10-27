// Graphics/Vulkan/Utils/CEVulkanImage.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanImage
        {
        public:
            CEVulkanImage ();
            ~CEVulkanImage ();

            bool Create ( CEVulkanContext * context, uint32_t width, uint32_t height,
                          VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
            bool CreateFromData ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                  VkFormat format, const void * data, VkImageUsageFlags usage );
            void Destroy ();

            void TransitionLayout ( VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout );
            void CopyFromBuffer ( VkCommandBuffer commandBuffer, VkBuffer buffer );

            VkImage GetImage () const { return m_Image; }
            VkImageView GetImageView () const { return m_ImageView; }
            VkDeviceMemory GetMemory () const { return m_Memory; }
            uint32_t GetWidth () const { return m_Width; }
            uint32_t GetHeight () const { return m_Height; }
            VkFormat GetFormat () const { return m_Format; }
            VkImageLayout GetLayout () const { return m_CurrentLayout; }

        private:
            bool CreateImageView ();

            CEVulkanContext * m_Context = nullptr;
            VkImage m_Image = VK_NULL_HANDLE;
            VkDeviceMemory m_Memory = VK_NULL_HANDLE;
            VkImageView m_ImageView = VK_NULL_HANDLE;

            uint32_t m_Width = 0;
            uint32_t m_Height = 0;
            VkFormat m_Format = VK_FORMAT_UNDEFINED;
            VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        };
    }