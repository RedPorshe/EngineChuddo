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

            // Запрещаем копирование
            CEVulkanBuffer ( const CEVulkanBuffer & ) = delete;
            CEVulkanBuffer & operator=( const CEVulkanBuffer & ) = delete;

            // Разрешаем перемещение
            CEVulkanBuffer ( CEVulkanBuffer && other ) noexcept;
            CEVulkanBuffer & operator=( CEVulkanBuffer && other ) noexcept;
            VkDeviceMemory GetMemory () const { return m_Memory; }

            bool Create (
                CEVulkanContext * context,
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties
            );

            void UploadData ( const void * data, VkDeviceSize size );
            void Destroy ();

            // Геттеры
            VkBuffer GetBuffer () const { return m_Buffer; }
            VkDeviceSize GetSize () const { return m_Size; }
            void * GetMappedData () const { return m_MappedData; }
            bool IsValid () const { return m_Buffer != VK_NULL_HANDLE; }

        private:
            CEVulkanContext * m_Context = nullptr;
            VkBuffer m_Buffer = VK_NULL_HANDLE;
            VkDeviceMemory m_Memory = VK_NULL_HANDLE;
            VkDeviceSize m_Size = 0;
            void * m_MappedData = nullptr;
            bool m_IsMapped = false;

            uint32_t FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties );
        };
    }