
#include "Graphics/Vulkan/Core/CEVulkanBuffer.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"
#include <stdexcept>
#include <cstring>

namespace CE
    {
    CEVulkanBuffer::CEVulkanBuffer () = default;

    CEVulkanBuffer::~CEVulkanBuffer ()
        {
        Destroy ();
        }

    CEVulkanBuffer::CEVulkanBuffer ( CEVulkanBuffer && other ) noexcept
        : m_Context ( other.m_Context )
        , m_Buffer ( other.m_Buffer )
        , m_Memory ( other.m_Memory )
        , m_Size ( other.m_Size )
        , m_MappedData ( other.m_MappedData )
        , m_IsMapped ( other.m_IsMapped )
        {
        other.m_Context = nullptr;
        other.m_Buffer = VK_NULL_HANDLE;
        other.m_Memory = VK_NULL_HANDLE;
        other.m_Size = 0;
        other.m_MappedData = nullptr;
        other.m_IsMapped = false;
        }

    CEVulkanBuffer & CEVulkanBuffer::operator=( CEVulkanBuffer && other ) noexcept
        {
        if (this != &other)
            {
            Destroy ();

            m_Context = other.m_Context;
            m_Buffer = other.m_Buffer;
            m_Memory = other.m_Memory;
            m_Size = other.m_Size;
            m_MappedData = other.m_MappedData;
            m_IsMapped = other.m_IsMapped;

            other.m_Context = nullptr;
            other.m_Buffer = VK_NULL_HANDLE;
            other.m_Memory = VK_NULL_HANDLE;
            other.m_Size = 0;
            other.m_MappedData = nullptr;
            other.m_IsMapped = false;
            }
        return *this;
        }

    bool CEVulkanBuffer::Create ( CEVulkanContext * context, VkDeviceSize size,
                                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties )
        {
        if (!context || !context->GetDevice ())
            {
            CE_CORE_ERROR ( "Invalid Vulkan context for buffer creation" );
            return false;
            }

        m_Context = context;
        m_Size = size;

        VkDevice device = m_Context->GetDevice ()->GetDevice();

        // Создаем буфер
        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer ( device, &bufferInfo, nullptr, &m_Buffer ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create Vulkan buffer" );
            return false;
            }

            // Выделяем память
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements ( device, m_Buffer, &memRequirements );

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType ( memRequirements.memoryTypeBits, properties );

        if (vkAllocateMemory ( device, &allocInfo, nullptr, &m_Memory ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to allocate memory for Vulkan buffer" );
            vkDestroyBuffer ( device, m_Buffer, nullptr );
            m_Buffer = VK_NULL_HANDLE;
            return false;
            }

        vkBindBufferMemory ( device, m_Buffer, m_Memory, 0 );

        // Если нужен CPU-доступ, маппим память
        if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
            if (vkMapMemory ( device, m_Memory, 0, size, 0, &m_MappedData ) == VK_SUCCESS)
                {
                m_IsMapped = true;
               
                }
            else
                {
                CE_CORE_WARN ( "Failed to map buffer memory" );
                }
            }

        CE_CORE_DEBUG ( "Vulkan buffer created successfully (size: {} bytes)", size );
        return true;
        }

    void CEVulkanBuffer::UploadData ( const void * data, VkDeviceSize size )
        {
        if (!m_IsMapped || !data || !m_Context)
            {
            CE_CORE_ERROR ( "Cannot upload data - buffer not properly initialized" );
            return;
            }

        if (size > m_Size)
            {
            CE_CORE_WARN ( "Upload size {} exceeds buffer size {}, clamping", size, m_Size );
            size = m_Size;
            }

        memcpy ( m_MappedData, data, size );

        // Если память не когерентная, нужно делать флаш
        VkMappedMemoryRange memoryRange {};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = m_Memory;
        memoryRange.size = VK_WHOLE_SIZE;
        vkFlushMappedMemoryRanges ( m_Context->GetDevice ()->GetDevice(), 1, &memoryRange);

        CE_CORE_DEBUG ( "Uploaded {} bytes to buffer", size );
        }

    void CEVulkanBuffer::Destroy ()
        {
        if (!m_Context || !m_Context->GetDevice ()->GetDevice())
            return;

        VkDevice device = m_Context->GetDevice ()->GetDevice ();

        if (m_IsMapped)
            {
            vkUnmapMemory ( device, m_Memory );
            m_IsMapped = false;
            m_MappedData = nullptr;
            }

        if (m_Buffer != VK_NULL_HANDLE)
            {
            vkDestroyBuffer ( device, m_Buffer, nullptr );
            m_Buffer = VK_NULL_HANDLE;
            }

        if (m_Memory != VK_NULL_HANDLE)
            {
            vkFreeMemory ( device, m_Memory, nullptr );
            m_Memory = VK_NULL_HANDLE;
            }

        m_Size = 0;
        m_Context = nullptr;

        CE_CORE_DEBUG ( "Vulkan buffer destroyed" );
        }

    uint32_t CEVulkanBuffer::FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties )
        {
        if (!m_Context)
            return 0;

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties ( m_Context->GetDevice()-> GetPhysicalDevice (), &memProperties );

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
            if (( typeFilter & ( 1 << i ) ) &&
                 ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties)
                {
                return i;
                }
            }

        CE_CORE_ERROR ( "Failed to find suitable memory type for buffer!" );
        return 0;
        }
    }