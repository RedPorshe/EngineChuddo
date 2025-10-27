// Graphics/Vulkan/Memory/CEVulkanBuffer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanBuffer
        {
        public:
            CEVulkanBuffer ();
            ~CEVulkanBuffer ();

            CEVulkanBuffer ( const CEVulkanBuffer & ) = delete;
            CEVulkanBuffer & operator=( const CEVulkanBuffer & ) = delete;

            CEVulkanBuffer ( CEVulkanBuffer && other ) noexcept;
            CEVulkanBuffer & operator=( CEVulkanBuffer && other ) noexcept;

            bool Create ( CEVulkanContext * context, VkDeviceSize size,
                          VkBufferUsageFlags usage, VkMemoryPropertyFlags properties );

            void UploadData ( const void * data, VkDeviceSize size, VkDeviceSize offset = 0 );
            void DownloadData ( void * data, VkDeviceSize size, VkDeviceSize offset = 0 );
            void Destroy ();

            void * Map ();
            void Unmap ();
            void Flush ( VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0 );

            VkBuffer GetBuffer () const { return m_Buffer; }
            VkDeviceMemory GetMemory () const { return m_Memory; }
            VkDeviceSize GetSize () const { return m_Size; }
            void * GetMappedData () const { return m_MappedData; }
            bool IsValid () const { return m_Buffer != VK_NULL_HANDLE; }
            bool IsMapped () const { return m_IsMapped; }

        private:
            uint32_t FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties );

            CEVulkanContext * m_Context = nullptr;
            VkBuffer m_Buffer = VK_NULL_HANDLE;
            VkDeviceMemory m_Memory = VK_NULL_HANDLE;
            VkDeviceSize m_Size = 0;
            void * m_MappedData = nullptr;
            bool m_IsMapped = false;
        };
    }