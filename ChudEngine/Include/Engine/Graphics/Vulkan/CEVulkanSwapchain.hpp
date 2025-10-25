// Runtime/Renderer/Vulkan/CEVulkanSwapchain.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "Core/Containers/CEArray.hpp"
#include "CEVulkanContext.hpp"

namespace CE
    {
    class CEWindow;

    class CEVulkanSwapchain
        {
        public:
            CEVulkanSwapchain ( CEVulkanContext * context );
            ~CEVulkanSwapchain ();

            bool Initialize ( CEWindow * window );
            void Shutdown ();
            void Recreate ( CEWindow * window );

            // Getters
            VkSwapchainKHR GetSwapchain () const { return Swapchain; }
            VkRenderPass GetRenderPass () const { return RenderPass; }
            VkFormat GetImageFormat () const { return ImageFormat; }
            VkExtent2D GetExtent () const { return Extent; }
            const CEArray<VkImage> & GetImages () const { return Images; }
            const CEArray<VkImageView> & GetImageViews () const { return ImageViews; }
            const CEArray<VkFramebuffer> & GetFramebuffers () const { return Framebuffers; }
            VkFramebuffer GetCurrentFramebuffer () const { return Framebuffers[ CurrentImageIndex ]; }
            uint32_t GetCurrentImageIndex () const { return CurrentImageIndex; }

            // Frame acquisition
            VkResult AcquireNextImage ( VkSemaphore imageAvailableSemaphore );
            VkResult SubmitCommandBuffer ( VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence );

            bool CreateUIRenderPass ();
           // VkRenderPass GetUIRenderPass () const { return UIRenderPass; }
            VkRenderPass GetUIRenderPass () const { return RenderPass; }

        private:
            void CreateSwapchain ( CEWindow * window );
            void CreateImageViews ();
            void CreateRenderPass ();
            void CreateFramebuffers ();
            void CreateDepthResources ();
            void CleanupSwapchain ();

            // Helper methods
            VkSurfaceFormatKHR ChooseSwapSurfaceFormat ( const CEArray<VkSurfaceFormatKHR> & availableFormats );
            VkPresentModeKHR ChooseSwapPresentMode ( const CEArray<VkPresentModeKHR> & availablePresentModes );
            VkExtent2D ChooseSwapExtent ( CEWindow * window, const VkSurfaceCapabilitiesKHR & capabilities );

            CEVulkanContext * Context = nullptr;

            // Swapchain objects
            VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
            CEArray<VkImage> Images;
            CEArray<VkImageView> ImageViews;
            VkFormat ImageFormat;
            VkExtent2D Extent;

            // Render pass and framebuffers
            VkRenderPass RenderPass = VK_NULL_HANDLE;
            CEArray<VkFramebuffer> Framebuffers;

            // Depth resources
            VkImage DepthImage = VK_NULL_HANDLE;
            VkDeviceMemory DepthImageMemory = VK_NULL_HANDLE;
            VkImageView DepthImageView = VK_NULL_HANDLE;

            // Current state
            uint32_t CurrentImageIndex = 0;
            VkRenderPass UIRenderPass = VK_NULL_HANDLE; // Добавляем если нужно отдельный
        };
    }