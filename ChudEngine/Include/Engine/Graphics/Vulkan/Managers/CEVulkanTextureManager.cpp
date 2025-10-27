#include "Graphics/Vulkan/Managers/CEVulkanTextureManager.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
        // CEVulkanTexture implementation
    CEVulkanTexture::CEVulkanTexture ()
        : m_Context ( nullptr )
        , m_Image ( VK_NULL_HANDLE )
        , m_ImageMemory ( VK_NULL_HANDLE )
        , m_ImageView ( VK_NULL_HANDLE )
        , m_Sampler ( VK_NULL_HANDLE )
        , m_Width ( 0 )
        , m_Height ( 0 )
        , m_Format ( VK_FORMAT_R8G8B8A8_SRGB )
        , m_CurrentLayout ( VK_IMAGE_LAYOUT_UNDEFINED )
        , m_Usage ( 0 )
        {
        }

    CEVulkanTexture::~CEVulkanTexture ()
        {
        Destroy ();
        }

    bool CEVulkanTexture::CreateFromFile ( CEVulkanContext * context, const std::string & filename )
        {
            // This would load an image file, create staging buffer, then create texture
            // For now, create a default placeholder texture
        CE_CORE_WARN ( "CreateFromFile not implemented, creating placeholder texture" );
        return CreateFromData ( context, 64, 64, VK_FORMAT_R8G8B8A8_SRGB, nullptr );
        }

    bool CEVulkanTexture::CreateFromData ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                           VkFormat format, const void * data, VkImageUsageFlags additionalUsage )
        {
        if (m_Image != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Texture already created" );
            return false;
            }

        m_Context = context;
        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | additionalUsage;

        try
            {
            if (!CreateImage ())
                {
                throw std::runtime_error ( "Failed to create texture image" );
                }

            if (!CreateImageView ())
                {
                throw std::runtime_error ( "Failed to create texture image view" );
                }

            if (!CreateSampler ())
                {
                throw std::runtime_error ( "Failed to create texture sampler" );
                }

            CE_CORE_DEBUG ( "Texture created successfully: {}x{}, format: {}", width, height, static_cast< int >( format ) );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create texture: {}", e.what () );
                Destroy ();
                return false;
                }
        }

    bool CEVulkanTexture::CreateRenderTarget ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                               VkFormat format, VkImageUsageFlags usage )
        {
        m_Context = context;
        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_Usage = usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        try
            {
            if (!CreateImage ())
                {
                throw std::runtime_error ( "Failed to create render target image" );
                }

            if (!CreateImageView ())
                {
                throw std::runtime_error ( "Failed to create render target image view" );
                }

                // Render targets typically don't need samplers
            CE_CORE_DEBUG ( "Render target created: {}x{}, format: {}", width, height, static_cast< int >( format ) );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create render target: {}", e.what () );
                Destroy ();
                return false;
                }
        }

    void CEVulkanTexture::Destroy ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            if (m_Sampler != VK_NULL_HANDLE)
                {
                vkDestroySampler ( device, m_Sampler, nullptr );
                m_Sampler = VK_NULL_HANDLE;
                }

            if (m_ImageView != VK_NULL_HANDLE)
                {
                vkDestroyImageView ( device, m_ImageView, nullptr );
                m_ImageView = VK_NULL_HANDLE;
                }

            if (m_Image != VK_NULL_HANDLE)
                {
                vkDestroyImage ( device, m_Image, nullptr );
                m_Image = VK_NULL_HANDLE;
                }

            if (m_ImageMemory != VK_NULL_HANDLE)
                {
                vkFreeMemory ( device, m_ImageMemory, nullptr );
                m_ImageMemory = VK_NULL_HANDLE;
                }
            }

        m_Width = 0;
        m_Height = 0;
        m_Format = VK_FORMAT_R8G8B8A8_SRGB;
        m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_Usage = 0;
        m_Context = nullptr;
        }

    void CEVulkanTexture::TransitionLayout ( VkCommandBuffer commandBuffer, VkImageLayout newLayout )
        {
        if (m_CurrentLayout == newLayout)
            {
            return;
            }

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = m_CurrentLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (m_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
        else if (m_CurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
        else
            {
            CE_CORE_WARN ( "Unsupported layout transition" );
            return;
            }

        vkCmdPipelineBarrier (
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        m_CurrentLayout = newLayout;
        }

    void CEVulkanTexture::CopyFromBuffer ( VkCommandBuffer commandBuffer, VkBuffer buffer )
        {
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { m_Width, m_Height, 1 };

        vkCmdCopyBufferToImage (
            commandBuffer,
            buffer,
            m_Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
        }

    bool CEVulkanTexture::CreateImage ()
        {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_Format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = m_Usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        VkResult result = vkCreateImage ( m_Context->GetDevice ()->GetDevice (), &imageInfo, nullptr, &m_Image );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create texture image" );
            return false;
            }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements ( m_Context->GetDevice ()->GetDevice (), m_Image, &memRequirements );

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Context->GetDevice ()->FindMemoryType (
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        result = vkAllocateMemory ( m_Context->GetDevice ()->GetDevice (), &allocInfo, nullptr, &m_ImageMemory );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to allocate texture memory" );
            return false;
            }

        vkBindImageMemory ( m_Context->GetDevice ()->GetDevice (), m_Image, m_ImageMemory, 0 );

        return true;
        }

    bool CEVulkanTexture::CreateImageView ()
        {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_Format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView ( m_Context->GetDevice ()->GetDevice (), &viewInfo, nullptr, &m_ImageView );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create texture image view" );
            return false;
            }

        return true;
        }

    bool CEVulkanTexture::CreateSampler ()
        {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = m_Context->GetDevice ()->GetPhysicalDeviceProperties ().limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkResult result = vkCreateSampler ( m_Context->GetDevice ()->GetDevice (), &samplerInfo, nullptr, &m_Sampler );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create texture sampler" );
            return false;
            }

        return true;
        }
    }

    // CEVulkanTextureManager implementation
namespace CE
    {
    CEVulkanTextureManager::CEVulkanTextureManager ( CEVulkanContext * context )
        : m_Context ( context )
        {
        }

    CEVulkanTextureManager::~CEVulkanTextureManager ()
        {
        Shutdown ();
        }

    bool CEVulkanTextureManager::Initialize ()
        {
        try
            {
            if (!CreateDefaultTextures ())
                {
                throw std::runtime_error ( "Failed to create default textures" );
                }

            CE_CORE_DEBUG ( "Texture manager initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize texture manager: {}", e.what () );
                return false;
                }
        }

    void CEVulkanTextureManager::Shutdown ()
        {
        m_Textures.clear ();
        m_DefaultTexture.reset ();
        m_NormalMapTexture.reset ();
        m_BlackTexture.reset ();
        m_WhiteTexture.reset ();

        CE_CORE_DEBUG ( "Texture manager shut down" );
        }

    std::shared_ptr<CEVulkanTexture> CEVulkanTextureManager::LoadTexture ( const std::string & filename )
        {
        auto it = m_Textures.find ( filename );
        if (it != m_Textures.end ())
            {
            return it->second;
            }

        auto texture = std::make_shared<CEVulkanTexture> ();
        if (texture->CreateFromFile ( m_Context, filename ))
            {
            m_Textures[ filename ] = texture;
            return texture;
            }

        CE_CORE_ERROR ( "Failed to load texture: {}", filename );
        return GetDefaultTexture ();
        }

    std::shared_ptr<CEVulkanTexture> CEVulkanTextureManager::CreateTexture ( const std::string & name, uint32_t width, uint32_t height,
                                                                             VkFormat format, const void * data )
        {
        auto it = m_Textures.find ( name );
        if (it != m_Textures.end ())
            {
            CE_CORE_WARN ( "Texture already exists: {}", name );
            return it->second;
            }

        auto texture = std::make_shared<CEVulkanTexture> ();
        if (texture->CreateFromData ( m_Context, width, height, format, data ))
            {
            m_Textures[ name ] = texture;
            return texture;
            }

        return nullptr;
        }

        // ... остальные методы TextureManager
    }