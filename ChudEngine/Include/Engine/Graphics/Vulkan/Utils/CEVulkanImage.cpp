#include "Graphics/Vulkan/Utils/CEVulkanImage.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanImage::CEVulkanImage ()
        : m_Context ( nullptr )
        , m_Image ( VK_NULL_HANDLE )
        , m_Memory ( VK_NULL_HANDLE )
        , m_ImageView ( VK_NULL_HANDLE )
        , m_Width ( 0 )
        , m_Height ( 0 )
        , m_Format ( VK_FORMAT_UNDEFINED )
        , m_CurrentLayout ( VK_IMAGE_LAYOUT_UNDEFINED )
        {
        }

    CEVulkanImage::~CEVulkanImage ()
        {
        Destroy ();
        }

    bool CEVulkanImage::Create ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                 VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties )
        {
        if (m_Image != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Image already created" );
            return false;
            }

        m_Context = context;
        m_Width = width;
        m_Height = height;
        m_Format = format;

        try
            {
            if (!CreateImage ())
                {
                throw std::runtime_error ( "Failed to create image" );
                }

            if (!CreateImageView ())
                {
                throw std::runtime_error ( "Failed to create image view" );
                }

            CE_CORE_DEBUG ( "Image created successfully: {}x{}, format: {}", width, height, static_cast< int >( format ) );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create image: {}", e.what () );
                Destroy ();
                return false;
                }
        }

    bool CEVulkanImage::CreateFromData ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                         VkFormat format, const void * data, VkImageUsageFlags usage )
        {
            // This would create a staging buffer, copy data, then create the image
            // For now, just create the image without data
        CE_CORE_WARN ( "CreateFromData not fully implemented, creating empty image" );
        return Create ( context, width, height, format, usage );
        }

    void CEVulkanImage::Destroy ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

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

            if (m_Memory != VK_NULL_HANDLE)
                {
                vkFreeMemory ( device, m_Memory, nullptr );
                m_Memory = VK_NULL_HANDLE;
                }
            }

        m_Width = 0;
        m_Height = 0;
        m_Format = VK_FORMAT_UNDEFINED;
        m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_Context = nullptr;
        }

    void CEVulkanImage::TransitionLayout ( VkCommandBuffer commandBuffer, VkImageLayout newLayout )
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
            throw std::invalid_argument ( "Unsupported layout transition" );
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

    void CEVulkanImage::CopyFromBuffer ( VkCommandBuffer commandBuffer, VkBuffer buffer )
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

    bool CEVulkanImage::CreateImage ()
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
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        VkResult result = vkCreateImage ( m_Context->GetDevice ()->GetDevice (), &imageInfo, nullptr, &m_Image );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create image" );
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

        result = vkAllocateMemory ( m_Context->GetDevice ()->GetDevice (), &allocInfo, nullptr, &m_Memory );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to allocate image memory" );
            return false;
            }

        vkBindImageMemory ( m_Context->GetDevice ()->GetDevice (), m_Image, m_Memory, 0 );

        return true;
        }

    bool CEVulkanImage::CreateImageView ()
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
    }