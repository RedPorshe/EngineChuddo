#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"

namespace CE
    {
    class VulkanDevice;
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

            bool Create ( CEVulkanContext * context, VkDeviceSize size,
                          VkBufferUsageFlags usage, VkMemoryPropertyFlags properties );

            void UploadData ( const void * data, VkDeviceSize size );
            void Destroy ();
            uint32_t FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties );
            // Геттеры
            VkBuffer GetBuffer () const { return m_Buffer; }
            VkDeviceMemory GetMemory () const { return m_Memory; }
            VkDeviceSize GetSize () const { return m_Size; }
            void * GetMappedData () const { return m_MappedData; }
            bool IsValid () const { return m_Buffer != VK_NULL_HANDLE; }

        private:
            VulkanDevice * m_Device = nullptr;
            VkBuffer m_Buffer = VK_NULL_HANDLE;
            VkDeviceMemory m_Memory = VK_NULL_HANDLE;
            VkDeviceSize m_Size = 0;
            CEVulkanContext m_Context;
            void * m_MappedData = nullptr;
            bool m_IsMapped = false;
        };
    }