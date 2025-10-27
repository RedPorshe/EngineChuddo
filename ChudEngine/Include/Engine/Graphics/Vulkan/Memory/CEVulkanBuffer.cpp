#include "Graphics/Vulkan/Memory/CEVulkanBuffer.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"
#include <cstring>

namespace CE
    {
    CEVulkanBuffer::CEVulkanBuffer ()
        : m_Context ( nullptr )
        , m_Buffer ( VK_NULL_HANDLE )
        , m_Memory ( VK_NULL_HANDLE )
        , m_Size ( 0 )
        , m_MappedData ( nullptr )
        , m_IsMapped ( false )
        {
        }

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
        if (m_Buffer != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Buffer already created" );
            return false;
            }

        m_Context = context;
        m_Size = size;

        if (m_Context == nullptr || m_Context->GetDevice () == nullptr)
            {
            CE_CORE_ERROR ( "Invalid Vulkan context" );
            return false;
            }

        try
            {
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkResult result = vkCreateBuffer ( m_Context->GetDevice ()->GetDevice (), &bufferInfo, nullptr, &m_Buffer );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create buffer" );
                }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements ( m_Context->GetDevice ()->GetDevice (), m_Buffer, &memRequirements );

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = FindMemoryType ( memRequirements.memoryTypeBits, properties );

            result = vkAllocateMemory ( m_Context->GetDevice ()->GetDevice (), &allocInfo, nullptr, &m_Memory );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to allocate buffer memory" );
                }

            vkBindBufferMemory ( m_Context->GetDevice ()->GetDevice (), m_Buffer, m_Memory, 0 );

            CE_CORE_DEBUG ( "Buffer created successfully: size={}", size );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create buffer: {}", e.what () );
                Destroy ();
                return false;
                }
        }

    void CEVulkanBuffer::UploadData ( const void * data, VkDeviceSize size, VkDeviceSize offset )
        {
        if (data == nullptr || size == 0)
            {
            CE_CORE_WARN ( "Invalid data for buffer upload" );
            return;
            }

        if (offset + size > m_Size)
            {
            CE_CORE_ERROR ( "Buffer upload exceeds buffer size" );
            return;
            }

        void * mappedData = Map ();
        if (mappedData)
            {
            memcpy ( static_cast< char * >( mappedData ) + offset, data, size );

            // If memory is not host coherent, we need to flush
            if (!m_IsMapped)
                {
                Flush ( size, offset );
                Unmap ();
                }
            }
        }

    void CEVulkanBuffer::DownloadData ( void * data, VkDeviceSize size, VkDeviceSize offset )
        {
        if (data == nullptr || size == 0)
            {
            CE_CORE_WARN ( "Invalid parameters for buffer download" );
            return;
            }

        if (offset + size > m_Size)
            {
            CE_CORE_ERROR ( "Buffer download exceeds buffer size" );
            return;
            }

        void * mappedData = Map ();
        if (mappedData)
            {
            memcpy ( data, static_cast< char * >( mappedData ) + offset, size );

            if (!m_IsMapped)
                {
                Unmap ();
                }
            }
        }

    void CEVulkanBuffer::Destroy ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            if (m_IsMapped && m_MappedData)
                {
                vkUnmapMemory ( device, m_Memory );
                m_MappedData = nullptr;
                m_IsMapped = false;
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
            }

        m_Size = 0;
        m_Context = nullptr;
        }

    void * CEVulkanBuffer::Map ()
        {
        if (m_IsMapped)
            {
            return m_MappedData;
            }

        if (m_Context && m_Context->GetDevice () && m_Memory != VK_NULL_HANDLE)
            {
            VkResult result = vkMapMemory ( m_Context->GetDevice ()->GetDevice (), m_Memory, 0, m_Size, 0, &m_MappedData );
            if (result == VK_SUCCESS)
                {
                m_IsMapped = true;
                return m_MappedData;
                }
            }

        CE_CORE_ERROR ( "Failed to map buffer memory" );
        return nullptr;
        }

    void CEVulkanBuffer::Unmap ()
        {
        if (m_IsMapped && m_Context && m_Context->GetDevice ())
            {
            vkUnmapMemory ( m_Context->GetDevice ()->GetDevice (), m_Memory );
            m_MappedData = nullptr;
            m_IsMapped = false;
            }
        }

    void CEVulkanBuffer::Flush ( VkDeviceSize size, VkDeviceSize offset )
        {
        if (m_Context && m_Context->GetDevice () && m_Memory != VK_NULL_HANDLE)
            {
            VkMappedMemoryRange memoryRange = {};
            memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.memory = m_Memory;
            memoryRange.offset = offset;
            memoryRange.size = size;

            vkFlushMappedMemoryRanges ( m_Context->GetDevice ()->GetDevice (), 1, &memoryRange );
            }
        }

    uint32_t CEVulkanBuffer::FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties )
        {
        if (m_Context && m_Context->GetDevice ())
            {
            return m_Context->GetDevice ()->FindMemoryType ( typeFilter, properties );
            }
        throw std::runtime_error ( "No valid device for memory type search" );
        }
    }