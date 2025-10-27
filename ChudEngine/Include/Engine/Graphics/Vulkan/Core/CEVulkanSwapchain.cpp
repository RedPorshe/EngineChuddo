#include "Graphics/Vulkan/Core/CEVulkanSwapchain.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Utils/Logger.hpp"
#include <algorithm>
#include <stdexcept>

namespace CE
    {
    CEVulkanSwapchain::CEVulkanSwapchain ()
        : m_Context ( nullptr )
        , m_Swapchain ( VK_NULL_HANDLE )
        , m_ImageFormat ( VK_FORMAT_UNDEFINED )
        , m_RenderPass ( VK_NULL_HANDLE )
        , m_DepthImage ( VK_NULL_HANDLE )
        , m_DepthImageMemory ( VK_NULL_HANDLE )
        , m_DepthImageView ( VK_NULL_HANDLE )
        , m_ColorImage ( VK_NULL_HANDLE )
        , m_ColorImageMemory ( VK_NULL_HANDLE )
        , m_ColorImageView ( VK_NULL_HANDLE )
        {
        }

    CEVulkanSwapchain::~CEVulkanSwapchain ()
        {
        Shutdown ();
        }

    bool CEVulkanSwapchain::Initialize ( CEVulkanContext * context, CEWindow * window, CEVulkanSwapchain * oldSwapchain )
        {
        if (m_Swapchain != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Swapchain already initialized" );
            return true;
            }

        m_Context = context;

        try
            {
            VkSwapchainKHR oldSwapchainHandle = oldSwapchain ? oldSwapchain->GetSwapchain () : VK_NULL_HANDLE;
            CreateSwapchain ( window, oldSwapchainHandle );
            CreateImageViews ();
            CreateDepthResources ();
            // УБРАЛИ CreateRenderPass() - это ответственность CEVulkanRenderPassManager
            CreateFramebuffers ();

            CE_CORE_DEBUG ( "Swapchain initialized successfully: {}x{}, {} images",
                            m_Extent.width, m_Extent.height, m_Images.size () );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize swapchain: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanSwapchain::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            CleanupSwapchain ();

            // УБРАЛИ уничтожение RenderPass - это ответственность CEVulkanRenderPassManager
            }

        m_Context = nullptr;
        }

    VkResult CEVulkanSwapchain::AcquireNextImage ( VkSemaphore imageAvailableSemaphore, uint32_t * imageIndex )
        {
        return vkAcquireNextImageKHR (
            m_Context->GetDevice ()->GetDevice (),
            m_Swapchain,
            UINT64_MAX,
            imageAvailableSemaphore,
            VK_NULL_HANDLE,
            imageIndex
        );
        }

    VkResult CEVulkanSwapchain::Present ( uint32_t imageIndex, VkSemaphore renderFinishedSemaphore )
        {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        return vkQueuePresentKHR ( m_Context->GetDevice ()->GetPresentQueue (), &presentInfo );
        }

    VkFramebuffer CEVulkanSwapchain::GetFramebuffer ( uint32_t index ) const
        {
        return ( index < m_Framebuffers.size () ) ? m_Framebuffers[ index ] : VK_NULL_HANDLE;
        }

    void CEVulkanSwapchain::CreateSwapchain ( CEWindow * window, VkSwapchainKHR oldSwapchain )
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for swapchain creation" );
            }

        VulkanDevice * device = m_Context->GetDevice ();
        VkPhysicalDevice physicalDevice = device->GetPhysicalDevice ();

        // Get swapchain capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( physicalDevice, m_Context->GetSurface (), &capabilities );

        // Choose surface format, present mode and extent
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, m_Context->GetSurface (), &formatCount, nullptr );
        std::vector<VkSurfaceFormatKHR> availableFormats ( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, m_Context->GetSurface (), &formatCount, availableFormats.data () );

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, m_Context->GetSurface (), &presentModeCount, nullptr );
        std::vector<VkPresentModeKHR> availablePresentModes ( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, m_Context->GetSurface (), &presentModeCount, availablePresentModes.data () );

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat ( availableFormats );
        VkPresentModeKHR presentMode = ChooseSwapPresentMode ( availablePresentModes );
        VkExtent2D extent = ChooseSwapExtent ( window, capabilities );

        // Determine number of images in swapchain
        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
            {
            imageCount = capabilities.maxImageCount;
            }

            // Create swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Context->GetSurface ();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device->GetQueueFamilyIndices ();
        uint32_t queueFamilyIndices [] = { indices.graphicsFamily.value (), indices.presentFamily.value () };

        if (indices.graphicsFamily != indices.presentFamily)
            {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
        else
            {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
            }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain;

        VkResult result = vkCreateSwapchainKHR ( device->GetDevice (), &createInfo, nullptr, &m_Swapchain );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create swap chain" );
            }

            // Get swapchain images
        vkGetSwapchainImagesKHR ( device->GetDevice (), m_Swapchain, &imageCount, nullptr );
        m_Images.resize ( imageCount );
        vkGetSwapchainImagesKHR ( device->GetDevice (), m_Swapchain, &imageCount, m_Images.data () );

        m_ImageFormat = surfaceFormat.format;
        m_Extent = extent;
        }

    void CEVulkanSwapchain::CreateImageViews ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for image view creation" );
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();
        m_ImageViews.resize ( m_Images.size () );

        for (size_t i = 0; i < m_Images.size (); i++)
            {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_Images[ i ];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_ImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView ( device, &createInfo, nullptr, &m_ImageViews[ i ] );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create image views" );
                }
            }
        }
      

    // Продолжение методов CEVulkanSwapchain из предыдущего сообщения

    void CEVulkanSwapchain::CreateFramebuffers ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for framebuffer creation" );
            }

            // Теперь RenderPass должен быть передан извне, но для обратной совместимости оставим проверку
        if (m_RenderPass == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Render pass not set for framebuffer creation" );
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();
        m_Framebuffers.resize ( m_ImageViews.size () );

        for (size_t i = 0; i < m_ImageViews.size (); i++)
            {
            std::array<VkImageView, 2> attachments = {
                m_ImageViews[ i ],
                m_DepthImageView
                };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = static_cast< uint32_t > ( attachments.size () );
            framebufferInfo.pAttachments = attachments.data ();
            framebufferInfo.width = m_Extent.width;
            framebufferInfo.height = m_Extent.height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &m_Framebuffers[ i ] );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create framebuffer" );
                }
            }
        }

    void CEVulkanSwapchain::CreateDepthResources ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for depth resource creation" );
            }

        VulkanDevice * device = m_Context->GetDevice ();
        VkFormat depthFormat = device->FindDepthFormat ();

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Extent.width;
        imageInfo.extent.height = m_Extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateImage ( device->GetDevice (), &imageInfo, nullptr, &m_DepthImage );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create depth image" );
            }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements ( device->GetDevice (), m_DepthImage, &memRequirements );

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->FindMemoryType ( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        result = vkAllocateMemory ( device->GetDevice (), &allocInfo, nullptr, &m_DepthImageMemory );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to allocate depth image memory" );
            }

        vkBindImageMemory ( device->GetDevice (), m_DepthImage, m_DepthImageMemory, 0 );

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_DepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView ( device->GetDevice (), &viewInfo, nullptr, &m_DepthImageView );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create depth image view" );
            }
        }

      // Добавим метод для установки RenderPass извне
    void CEVulkanSwapchain::SetRenderPass ( VkRenderPass renderPass )
        {
        if (m_RenderPass != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Render pass already set, recreating framebuffers..." );
            // Пересоздаем framebuffers с новым render pass
            if (!m_Framebuffers.empty ())
                {
                for (auto framebuffer : m_Framebuffers)
                    {
                    if (framebuffer != VK_NULL_HANDLE)
                        {
                        vkDestroyFramebuffer ( m_Context->GetDevice ()->GetDevice (), framebuffer, nullptr );
                        }
                    }
                m_Framebuffers.clear ();
                }
            }

        m_RenderPass = renderPass;

        if (m_RenderPass != VK_NULL_HANDLE && !m_ImageViews.empty ())
            {
            CreateFramebuffers ();
            }
        }

    void CEVulkanSwapchain::CreateMultisampleResources ()
        {
            // Implementation for multisampling would go here
            // Currently using single sample for simplicity
        }

    void CEVulkanSwapchain::CleanupSwapchain ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            return;
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();

        if (m_DepthImageView != VK_NULL_HANDLE)
            {
            vkDestroyImageView ( device, m_DepthImageView, nullptr );
            m_DepthImageView = VK_NULL_HANDLE;
            }

        if (m_DepthImage != VK_NULL_HANDLE)
            {
            vkDestroyImage ( device, m_DepthImage, nullptr );
            m_DepthImage = VK_NULL_HANDLE;
            }

        if (m_DepthImageMemory != VK_NULL_HANDLE)
            {
            vkFreeMemory ( device, m_DepthImageMemory, nullptr );
            m_DepthImageMemory = VK_NULL_HANDLE;
            }

        for (auto framebuffer : m_Framebuffers)
            {
            if (framebuffer != VK_NULL_HANDLE)
                {
                vkDestroyFramebuffer ( device, framebuffer, nullptr );
                }
            }
        m_Framebuffers.clear ();

        for (auto imageView : m_ImageViews)
            {
            if (imageView != VK_NULL_HANDLE)
                {
                vkDestroyImageView ( device, imageView, nullptr );
                }
            }
        m_ImageViews.clear ();

        if (m_Swapchain != VK_NULL_HANDLE)
            {
            vkDestroySwapchainKHR ( device, m_Swapchain, nullptr );
            m_Swapchain = VK_NULL_HANDLE;
            }
        }

    VkSurfaceFormatKHR CEVulkanSwapchain::ChooseSwapSurfaceFormat ( const std::vector<VkSurfaceFormatKHR> & availableFormats )
        {
        for (const auto & availableFormat : availableFormats)
            {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                 availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                return availableFormat;
                }
            }

        return availableFormats[ 0 ];
        }

    VkPresentModeKHR CEVulkanSwapchain::ChooseSwapPresentMode ( const std::vector<VkPresentModeKHR> & availablePresentModes )
        {
        for (const auto & availablePresentMode : availablePresentModes)
            {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                return availablePresentMode;
                }
            }

        return VK_PRESENT_MODE_FIFO_KHR;
        }

    VkExtent2D CEVulkanSwapchain::ChooseSwapExtent ( CEWindow * window, const VkSurfaceCapabilitiesKHR & capabilities )
        {
        if (capabilities.currentExtent.width != UINT32_MAX)
            {
            return capabilities.currentExtent;
            }
        else
            {
            int width, height;
            window->GetFramebufferSize ( &width, &height );

            VkExtent2D actualExtent = {
                static_cast< uint32_t >( width ),
                static_cast< uint32_t >( height )
                };

            actualExtent.width = std::clamp ( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
            actualExtent.height = std::clamp ( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

            return actualExtent;
            }
        }
    }