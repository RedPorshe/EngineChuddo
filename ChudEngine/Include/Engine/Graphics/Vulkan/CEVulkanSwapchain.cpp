#include "CEVulkanSwapchain.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Core/Logger.h"
#include <stdexcept>
#include <algorithm>

namespace CE
    {
    CEVulkanSwapchain::CEVulkanSwapchain ( CEVulkanContext * context )
        : Context ( context )
        {
        CE_CORE_DEBUG ( "Vulkan swapchain created" );
        }

    CEVulkanSwapchain::~CEVulkanSwapchain ()
        {
        Shutdown ();
        }

    bool CEVulkanSwapchain::CreateUIRenderPass ()
        {
        auto device = Context->GetDevice ();

        // UI обычно требует blending
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = ImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // UI поверх существующего содержимого
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass ( device, &renderPassInfo, nullptr, &UIRenderPass ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create UI render pass!" );
            return false;
            }

        CE_CORE_DEBUG ( "UI render pass created successfully" );
        return true;
        }


    bool CEVulkanSwapchain::Initialize ( CEWindow * window )
        {
        try
            {
            CreateSwapchain ( window );
            CreateImageViews ();
            CreateRenderPass ();
            CreateDepthResources ();
            CreateFramebuffers ();

            CE_CORE_DEBUG ( "Vulkan swapchain initialized successfully" );
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

    void CEVulkanSwapchain::Recreate ( CEWindow * window )
        {
        CE_CORE_DEBUG ( "Recreating swapchain..." );

        vkDeviceWaitIdle ( Context->GetDevice () );
        CleanupSwapchain ();
        Initialize ( window );
        }

    void CEVulkanSwapchain::CreateSwapchain ( CEWindow * window )
        {
        auto physicalDevice = Context->GetPhysicalDevice ();
        auto device = Context->GetDevice ();
        auto surface = Context->GetSurface ();

        // Get swapchain capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( physicalDevice, surface, &capabilities );

        // Get available formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, nullptr );
        CEArray<VkSurfaceFormatKHR> availableFormats;
        availableFormats.Resize ( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( physicalDevice, surface, &formatCount, availableFormats.RawData () );

        // Get available present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR ( physicalDevice, surface, &presentModeCount, nullptr );
        CEArray<VkPresentModeKHR> availablePresentModes;
        availablePresentModes.Resize ( presentModeCount );
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
        auto queueIndices = Context->GetQueueFamilyIndices ();
        uint32_t queueFamilyIndices [] = { queueIndices.graphicsFamily.value (), queueIndices.presentFamily.value () };

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
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR ( device, &createInfo, nullptr, &Swapchain ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create swap chain!" );
            }

            // Get swapchain images
        vkGetSwapchainImagesKHR ( device, Swapchain, &imageCount, nullptr );
        Images.Resize ( imageCount );
        vkGetSwapchainImagesKHR ( device, Swapchain, &imageCount, Images.RawData () );

        ImageFormat = surfaceFormat.format;
        Extent = extent;

        CE_CORE_DEBUG ( "Swapchain created with {} images", imageCount );
        }

    void CEVulkanSwapchain::CreateImageViews ()
        {
        auto device = Context->GetDevice ();
        ImageViews.Resize ( Images.Size () );

        for (uint64 i = 0; i < Images.Size (); i++)
            {
            VkImageViewCreateInfo createInfo {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = Images[ i ];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = ImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView ( device, &createInfo, nullptr, &ImageViews[ i ] ) != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create image views!" );
                }
            }

        CE_CORE_DEBUG ( "Image views created" );
        }

    void CEVulkanSwapchain::CreateRenderPass ()
        {
        auto device = Context->GetDevice ();

        // ИСПРАВЛЯЕМ: Вместо std::format используем простой вывод
        CE_CORE_DEBUG ( "Creating render pass" );
        CE_CORE_DEBUG ( "Creating render pass with format: {}", Context->FormatToString ( ImageFormat ) );
        // Color attachment
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = ImageFormat;
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

        // УПРОЩАЕМ: Временно убираем depth attachment
        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = nullptr; // Без depth attachment

        // Dependency
        VkSubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Create render pass
        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1; // Только color attachment
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass ( device, &renderPassInfo, nullptr, &RenderPass ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create render pass!" );
            }

        CE_CORE_DEBUG ( "Render pass created successfully" );
        }

    void CEVulkanSwapchain::CreateFramebuffers ()
        {
        auto device = Context->GetDevice ();
        Framebuffers.Resize ( ImageViews.Size () );

        for (uint64 i = 0; i < ImageViews.Size (); i++)
            {
            VkImageView attachments[ 2 ] = {
                ImageViews[ i ],
                DepthImageView
                };

            VkFramebufferCreateInfo framebufferInfo {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = RenderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = Extent.width;
            framebufferInfo.height = Extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &Framebuffers[ i ] ) != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create framebuffer!" );
                }
            }

        CE_CORE_DEBUG ( "Framebuffers created" );
        }

    void CEVulkanSwapchain::CreateDepthResources ()
        {
        auto device = Context->GetDevice ();
        VkFormat depthFormat = Context->FindDepthFormat ();

        // Create depth image
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = Extent.width;
        imageInfo.extent.height = Extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage ( device, &imageInfo, nullptr, &DepthImage ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create depth image!" );
            }

            // Allocate memory for depth image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements ( device, DepthImage, &memRequirements );

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Context->FindMemoryType ( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if (vkAllocateMemory ( device, &allocInfo, nullptr, &DepthImageMemory ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to allocate depth image memory!" );
            }

        vkBindImageMemory ( device, DepthImage, DepthImageMemory, 0 );

        // Create depth image view
        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = DepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView ( device, &viewInfo, nullptr, &DepthImageView ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create depth image view!" );
            }

        CE_CORE_DEBUG ( "Depth resources created" );
        }

    VkResult CEVulkanSwapchain::AcquireNextImage ( VkSemaphore imageAvailableSemaphore )
        {
        auto device = Context->GetDevice ();
        VkResult result = vkAcquireNextImageKHR (
            device,
            Swapchain,
            UINT64_MAX,
            imageAvailableSemaphore,
            VK_NULL_HANDLE,
            &CurrentImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
            CE_CORE_DEBUG ( "Swapchain out of date during image acquisition" );
            return result;
            }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
            CE_CORE_ERROR ( "Failed to acquire swap chain image: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to acquire swap chain image!" );
            }

        return result;
        }

    VkResult CEVulkanSwapchain::SubmitCommandBuffer ( VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence )
                {

                

        auto graphicsQueue = Context->GetGraphicsQueue ();
        auto presentQueue = Context->GetPresentQueue ();

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores [] = { waitSemaphore };
        VkPipelineStageFlags waitStages [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores [] = { signalSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;


        VkResult submitResult = vkQueueSubmit ( graphicsQueue, 1, &submitInfo, fence );
       
        if (submitResult != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to submit draw command buffer: {}", static_cast< int >( submitResult ) );
            return submitResult;
            }

        VkPresentInfoKHR presentInfo {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains [] = { Swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &CurrentImageIndex;
        presentInfo.pResults = nullptr;

        VkResult presentResult = vkQueuePresentKHR ( presentQueue, &presentInfo );

        if (presentResult == VK_SUCCESS)
            {
           
            }
        else if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
            {
            CE_CORE_DEBUG ( "Swapchain out of date during presentation" );
            return presentResult;
            }
        else if (presentResult != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to present swap chain image: {}", static_cast< int >( presentResult ) );
            throw std::runtime_error ( "Failed to present swap chain image!" );
            }

        return presentResult;
        }

        // Helper methods
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

            // Проверка минимального размера
            if (width == 0 || height == 0)
                {
                    // Возвращаем минимально допустимый размер
                return capabilities.minImageExtent;
                }

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
        auto device = Context->GetDevice ();

        if (DepthImageView != VK_NULL_HANDLE)
            {
            vkDestroyImageView ( device, DepthImageView, nullptr );
            DepthImageView = VK_NULL_HANDLE;
            }

        if (DepthImage != VK_NULL_HANDLE)
            {
            vkDestroyImage ( device, DepthImage, nullptr );
            DepthImage = VK_NULL_HANDLE;
            }

        if (DepthImageMemory != VK_NULL_HANDLE)
            {
            vkFreeMemory ( device, DepthImageMemory, nullptr );
            DepthImageMemory = VK_NULL_HANDLE;
            }

        for (auto framebuffer : Framebuffers)
            {
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            }
        Framebuffers.Clear ();

        if (RenderPass != VK_NULL_HANDLE)
            {
            vkDestroyRenderPass ( device, RenderPass, nullptr );
            RenderPass = VK_NULL_HANDLE;
            }

        for (auto imageView : ImageViews)
            {
            vkDestroyImageView ( device, imageView, nullptr );
            }
        ImageViews.Clear ();

        if (Swapchain != VK_NULL_HANDLE)
            {
            vkDestroySwapchainKHR ( device, Swapchain, nullptr );
            Swapchain = VK_NULL_HANDLE;
            }

        CE_CORE_DEBUG ( "Swapchain cleaned up" );
        }
    }