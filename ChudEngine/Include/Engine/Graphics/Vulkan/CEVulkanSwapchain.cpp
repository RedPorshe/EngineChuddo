// Runtime/Renderer/Vulkan/CEVulkanSwapchain.cpp
#include "CEVulkanSwapchain.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Core/Logger.h"
#include <algorithm>
#include <stdexcept>
#include <array>

namespace CE
    {
    CEVulkanSwapchain::CEVulkanSwapchain ()
        {
        CE_CORE_DEBUG ( "Vulkan swapchain created" );
        }

    CEVulkanSwapchain::~CEVulkanSwapchain ()
        {
        Shutdown ();
        }

    bool CEVulkanSwapchain::Initialize ( CEVulkanContext * context, CEWindow * window, CEVulkanSwapchain * oldSwapchain )
        {
        m_Context = context;

        try
            {
            VkSwapchainKHR oldSwapchainHandle = oldSwapchain ? oldSwapchain->GetSwapchain () : VK_NULL_HANDLE;
            CreateSwapchain ( window, oldSwapchainHandle );
            CreateImageViews ();
            CreateRenderPass ();
            CreateDepthResources ();
            CreateFramebuffers ();

            CE_CORE_DEBUG ( "Vulkan swapchain initialized successfully with {} images", m_Images.Size () );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan swapchain: {}", e.what () );
                return false;
                }
        }

    void CEVulkanSwapchain::Shutdown ()
        {
        CleanupSwapchain ();
        CE_CORE_DEBUG ( "Vulkan swapchain shutdown complete" );
        }

    void CEVulkanSwapchain::CreateSwapchain ( CEWindow * window, VkSwapchainKHR oldSwapchain )
        {
        auto physicalDevice = m_Context->GetPhysicalDevice ();
        auto device = m_Context->GetDevice ();
        auto surface = m_Context->GetSurface ();

        // Get swapchain capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( physicalDevice, surface, &capabilities );

        // Get available formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, nullptr );
        CEArray<VkSurfaceFormatKHR> availableFormats ( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, availableFormats.RawData () );

        // Get available present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, nullptr );
        CEArray<VkPresentModeKHR> availablePresentModes ( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, availablePresentModes.RawData () );

        // Choose swapchain settings
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
        VkSwapchainCreateInfoKHR createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // Set up queue families
        auto queueIndices = m_Context->GetQueueFamilyIndices ();
        uint32_t queueFamilyIndices [] = {
            queueIndices.graphicsFamily.value (),
            queueIndices.presentFamily.value ()
            };

        if (queueIndices.graphicsFamily != queueIndices.presentFamily)
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

        if (vkCreateSwapchainKHR ( device, &createInfo, nullptr, &m_Swapchain ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create swap chain!" );
            }

            // Get swapchain images
        vkGetSwapchainImagesKHR ( device, m_Swapchain, &imageCount, nullptr );
        m_Images.Resize ( imageCount );
        vkGetSwapchainImagesKHR ( device, m_Swapchain, &imageCount, m_Images.RawData () );

        m_ImageFormat = surfaceFormat.format;
        m_Extent = extent;

        CE_CORE_DEBUG ( "Swapchain created with {} images", imageCount );
        }

    void CEVulkanSwapchain::CreateImageViews ()
        {
        auto device = m_Context->GetDevice ();
        m_ImageViews.Resize ( m_Images.Size () );

        for (uint64_t i = 0; i < m_Images.Size (); i++)
            {
            VkImageViewCreateInfo createInfo {};
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

            if (vkCreateImageView ( device, &createInfo, nullptr, &m_ImageViews[ i ] ) != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create image views!" );
                }
            }

        CE_CORE_DEBUG ( "Image views created" );
        }

    void CEVulkanSwapchain::CreateRenderPass ()
        {
        auto device = m_Context->GetDevice ();

        // Color attachment
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = m_ImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Depth attachment
        VkAttachmentDescription depthAttachment {};
        depthAttachment.format = m_Context->FindDepthFormat ();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Subpass
        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // Dependency
        VkSubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // Create render pass
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size () );
        renderPassInfo.pAttachments = attachments.data ();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass ( device, &renderPassInfo, nullptr, &m_RenderPass ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create render pass!" );
            }

        CE_CORE_DEBUG ( "Render pass created successfully" );
        }

    void CEVulkanSwapchain::CreateFramebuffers ()
        {
        auto device = m_Context->GetDevice ();
        m_Framebuffers.Resize ( m_ImageViews.Size () );

        for (uint64_t i = 0; i < m_ImageViews.Size (); i++)
            {
            std::array<VkImageView, 2> attachments = {
                m_ImageViews[ i ],
                m_DepthImageView
                };

            VkFramebufferCreateInfo framebufferInfo {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = static_cast< uint32_t > ( attachments.size () );
            framebufferInfo.pAttachments = attachments.data ();
            framebufferInfo.width = m_Extent.width;
            framebufferInfo.height = m_Extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &m_Framebuffers[ i ] ) != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create framebuffer!" );
                }
            }

        CE_CORE_DEBUG ( "Framebuffers created" );
        }

    void CEVulkanSwapchain::CreateDepthResources ()
        {
        auto device = m_Context->GetDevice ();
        VkFormat depthFormat = m_Context->FindDepthFormat ();

        // Create depth image
        VkImageCreateInfo imageInfo {};
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

        if (vkCreateImage ( device, &imageInfo, nullptr, &m_DepthImage ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create depth image!" );
            }

            // Allocate memory for depth image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements ( device, m_DepthImage, &memRequirements );

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Context->FindMemoryType ( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if (vkAllocateMemory ( device, &allocInfo, nullptr, &m_DepthImageMemory ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to allocate depth image memory!" );
            }

        vkBindImageMemory ( device, m_DepthImage, m_DepthImageMemory, 0 );

        // Create depth image view
        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_DepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView ( device, &viewInfo, nullptr, &m_DepthImageView ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create depth image view!" );
            }

        CE_CORE_DEBUG ( "Depth resources created" );
        }

    VkResult CEVulkanSwapchain::AcquireNextImage ( VkSemaphore imageAvailableSemaphore, uint32_t * imageIndex )
        {
        auto device = m_Context->GetDevice ();
        return vkAcquireNextImageKHR (
            device,
            m_Swapchain,
            UINT64_MAX,
            imageAvailableSemaphore,
            VK_NULL_HANDLE,
            imageIndex
        );
        }

    VkResult CEVulkanSwapchain::Present ( uint32_t imageIndex, VkSemaphore renderFinishedSemaphore )
        {
        auto presentQueue = m_Context->GetPresentQueue ();

        VkPresentInfoKHR presentInfo {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

        VkSwapchainKHR swapChains [] = { m_Swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        return vkQueuePresentKHR ( presentQueue, &presentInfo );
        }

        // Helper methods implementation...
    VkSurfaceFormatKHR CEVulkanSwapchain::ChooseSwapSurfaceFormat ( const CEArray<VkSurfaceFormatKHR> & availableFormats )
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

    VkPresentModeKHR CEVulkanSwapchain::ChooseSwapPresentMode ( const CEArray<VkPresentModeKHR> & availablePresentModes )
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
            glfwGetFramebufferSize ( window->GetNativeWindow (), &width, &height );

            VkExtent2D actualExtent = {
                static_cast< uint32_t >( width ),
                static_cast< uint32_t >( height )
                };

            actualExtent.width = std::clamp ( actualExtent.width,
                                              capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
            actualExtent.height = std::clamp ( actualExtent.height,
                                               capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

            return actualExtent;
            }
        }

    void CEVulkanSwapchain::CleanupSwapchain ()
        {
        auto device = m_Context ? m_Context->GetDevice () : VK_NULL_HANDLE;
        if (!device) return;

        for (auto framebuffer : m_Framebuffers)
            {
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            }
        m_Framebuffers.Clear ();

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

        for (auto imageView : m_ImageViews)
            {
            vkDestroyImageView ( device, imageView, nullptr );
            }
        m_ImageViews.Clear ();

        if (m_RenderPass != VK_NULL_HANDLE)
            {
            vkDestroyRenderPass ( device, m_RenderPass, nullptr );
            m_RenderPass = VK_NULL_HANDLE;
            }

        if (m_Swapchain != VK_NULL_HANDLE)
            {
            vkDestroySwapchainKHR ( device, m_Swapchain, nullptr );
            m_Swapchain = VK_NULL_HANDLE;
            }

        CE_CORE_DEBUG ( "Swapchain cleaned up" );
        }
    }