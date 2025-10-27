// Runtime/Renderer/Vulkan/CEVulkanResourceManager.cpp
#include "CEVulkanResourceManager.hpp"
#include "CEVulkanContext.hpp"

namespace CE
    {
    CEVulkanResourceManager::CEVulkanResourceManager ( CEVulkanContext * context )
        : m_Context ( context )
        {
        CE_CORE_DEBUG ( "Vulkan resource manager created" );
        }

    CEVulkanResourceManager::~CEVulkanResourceManager ()
        {
        Cleanup ();
        }

    std::shared_ptr<CEVulkanBuffer> CEVulkanResourceManager::CreateBuffer (
        const std::string & name,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties )
        {
            // Проверяем не существует ли уже буфер с таким именем
        auto it = m_Buffers.find ( name );
        if (it != m_Buffers.end ())
            {
            CE_CORE_WARN ( "Buffer '{}' already exists", name );
            return it->second;
            }

            // Создаем новый буфер
        auto buffer = std::make_shared<CEVulkanBuffer> ();
        if (!buffer->Create ( m_Context, size, usage, properties ))
            {
            CE_CORE_ERROR ( "Failed to create buffer '{}'", name );
            return nullptr;
            }

            // Сохраняем
        m_Buffers[ name ] = buffer;
        m_TotalAllocatedMemory += size;
        m_BufferCount++;

        CE_CORE_DEBUG ( "Buffer '{}' created ({} bytes)", name, size );
        return buffer;
        }

    std::shared_ptr<CEVulkanBuffer> CEVulkanResourceManager::GetBuffer ( const std::string & name )
        {
        auto it = m_Buffers.find ( name );
        if (it != m_Buffers.end ())
            {
            return it->second;
            }

        CE_CORE_WARN ( "Buffer '{}' not found", name );
        return nullptr;
        }

    void CEVulkanResourceManager::DestroyBuffer ( const std::string & name )
        {
        auto it = m_Buffers.find ( name );
        if (it != m_Buffers.end ())
            {
            if (it->second->IsValid ())
                {
                m_TotalAllocatedMemory -= it->second->GetSize ();
                }
            m_Buffers.erase ( it );
            CE_CORE_DEBUG ( "Buffer '{}' destroyed", name );
            m_BufferCount--;
            }
        }

    void CEVulkanResourceManager::PrintMemoryStatistics () const
        {
        CE_CORE_DEBUG ( "=== Vulkan Resource Manager Statistics ===" );
        CE_CORE_DEBUG ( "Total buffers: {}", m_BufferCount );
        CE_CORE_DEBUG ( "Total allocated memory: {} bytes", m_TotalAllocatedMemory );
        CE_CORE_DEBUG ( "Buffers:" );

        for (const auto & [name, buffer] : m_Buffers)
            {
            CE_CORE_DEBUG ( "  - {}: {} bytes", name, buffer->GetSize () );
            }
        }

    void CEVulkanResourceManager::Cleanup ()
        {
        CE_CORE_DEBUG ( "Cleaning up resource manager..." );

        for (auto & [name, buffer] : m_Buffers)
            {
            if (buffer->IsValid ())
                {
                buffer->Destroy ();
                }
            }
        m_Buffers.clear ();
        m_TotalAllocatedMemory = 0;
        m_BufferCount = 0;

        CE_CORE_DEBUG ( "Resource manager cleanup complete" );
        }
    }