#include "Graphics/Vulkan/Managers/CEVulkanResourceManager.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Memory/CEVulkanBuffer.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanResourceManager::CEVulkanResourceManager ( CEVulkanContext * context )
        : m_Context ( context )
        , m_TotalBufferMemory ( 0 )
        , m_TotalImageMemory ( 0 )
        {
        }

    CEVulkanResourceManager::~CEVulkanResourceManager ()
        {
        Cleanup ();
        }

    std::shared_ptr<CEVulkanBuffer> CEVulkanResourceManager::CreateBuffer (
        const std::string & name, VkDeviceSize size,
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties )
        {
            // Check if buffer already exists
        auto it = m_Buffers.find ( name );
        if (it != m_Buffers.end ())
            {
            CE_CORE_WARN ( "Buffer already exists: {}", name );
            return it->second;
            }

        try
            {
            auto buffer = std::make_shared<CEVulkanBuffer> ();
            if (buffer->Create ( m_Context, size, usage, properties ))
                {
                m_Buffers[ name ] = buffer;
                m_TotalBufferMemory += size;
                CE_CORE_DEBUG ( "Created buffer: {} (size: {})", name, size );
                return buffer;
                }
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create buffer {}: {}", name, e.what () );
                }

            return nullptr;
        }

    std::shared_ptr<CEVulkanBuffer> CEVulkanResourceManager::GetBuffer ( const std::string & name )
        {
        auto it = m_Buffers.find ( name );
        if (it != m_Buffers.end ())
            {
            return it->second;
            }

        CE_CORE_WARN ( "Buffer not found: {}", name );
        return nullptr;
        }

    void CEVulkanResourceManager::DestroyBuffer ( const std::string & name )
        {
        auto it = m_Buffers.find ( name );
        if (it != m_Buffers.end ())
            {
            if (it->second)
                {
                m_TotalBufferMemory -= it->second->GetSize ();
                }
            m_Buffers.erase ( it );
            CE_CORE_DEBUG ( "Destroyed buffer: {}", name );
            }
        else
            {
            CE_CORE_WARN ( "Buffer not found for destruction: {}", name );
            }
        }

    std::shared_ptr<CEVulkanImage> CEVulkanResourceManager::CreateImage (
        const std::string & name, uint32_t width, uint32_t height,
        VkFormat format, VkImageUsageFlags usage )
        {
            // Check if image already exists
        auto it = m_Images.find ( name );
        if (it != m_Images.end ())
            {
            CE_CORE_WARN ( "Image already exists: {}", name );
            return it->second;
            }

        try
            {
            auto image = std::make_shared<CEVulkanImage> ();
            if (image->Create ( m_Context, width, height, format, usage ))
                {
                m_Images[ name ] = image;
                // Estimate memory usage (this is approximate)
                VkDeviceSize estimatedSize = width * height * 4; // 4 bytes per pixel
                m_TotalImageMemory += estimatedSize;
                CE_CORE_DEBUG ( "Created image: {} ({}x{}, format: {})", name, width, height, static_cast< int >( format ) );
                return image;
                }
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create image {}: {}", name, e.what () );
                }

            return nullptr;
        }

    std::shared_ptr<CEVulkanImage> CEVulkanResourceManager::GetImage ( const std::string & name )
        {
        auto it = m_Images.find ( name );
        if (it != m_Images.end ())
            {
            return it->second;
            }

        CE_CORE_WARN ( "Image not found: {}", name );
        return nullptr;
        }

    void CEVulkanResourceManager::DestroyImage ( const std::string & name )
        {
        auto it = m_Images.find ( name );
        if (it != m_Images.end ())
            {
// Estimate memory usage for removal
            auto & image = it->second;
            if (image)
                {
                VkDeviceSize estimatedSize = 0; // We'd need to track this properly
                m_TotalImageMemory -= estimatedSize;
                }
            m_Images.erase ( it );
            CE_CORE_DEBUG ( "Destroyed image: {}", name );
            }
        else
            {
            CE_CORE_WARN ( "Image not found for destruction: {}", name );
            }
        }

    void CEVulkanResourceManager::Cleanup ()
        {
        m_Buffers.clear ();
        m_Images.clear ();
        m_TotalBufferMemory = 0;
        m_TotalImageMemory = 0;

        CE_CORE_DEBUG ( "Resource manager cleaned up" );
        }

    void CEVulkanResourceManager::PrintMemoryStatistics () const
        {
        CE_CORE_INFO ( "=== Resource Manager Memory Statistics ===" );
        CE_CORE_INFO ( "Buffers: {} (Total memory: {} bytes)", m_Buffers.size (), m_TotalBufferMemory );
        CE_CORE_INFO ( "Images: {} (Estimated memory: {} bytes)", m_Images.size (), m_TotalImageMemory );
        CE_CORE_INFO ( "Total managed memory: {} bytes", m_TotalBufferMemory + m_TotalImageMemory );

        // Detailed buffer information
        for (const auto & [name, buffer] : m_Buffers)
            {
            if (buffer)
                {
                CE_CORE_INFO ( "  Buffer '{}': {} bytes", name, buffer->GetSize () );
                }
            }
        }
    }