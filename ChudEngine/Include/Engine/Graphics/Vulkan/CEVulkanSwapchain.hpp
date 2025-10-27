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
            CEVulkanSwapchain ();
            ~CEVulkanSwapchain ();

            bool Initialize ( CEVulkanContext * context, CEWindow * window, CEVulkanSwapchain * oldSwapchain = nullptr );
            void Shutdown ();

            // Getters
            VkSwapchainKHR GetSwapchain () const { return m_Swapchain; }
            VkRenderPass GetRenderPass () const { return m_RenderPass; }
            VkFormat GetImageFormat () const { return m_ImageFormat; }
            VkExtent2D GetExtent () const { return m_Extent; }
            const CEArray<VkImage> & GetImages () const { return m_Images; }
            const CEArray<VkImageView> & GetImageViews () const { return m_ImageViews; }
            const CEArray<VkFramebuffer> & GetFramebuffers () const { return m_Framebuffers; }
            VkFramebuffer GetFramebuffer ( uint32_t index ) const {
                return ( index < m_Framebuffers.Size () ) ? m_Framebuffers[ index ] : VK_NULL_HANDLE;
                }
            uint32_t GetImageCount () const { return static_cast< uint32_t > ( m_Images.Size () ); }
            uint32_t GetMaxFramesInFlight () const { return 2; } // Стандартное значение

            // Frame operations
            VkResult AcquireNextImage ( VkSemaphore imageAvailableSemaphore, uint32_t * imageIndex );
            VkResult Present ( uint32_t imageIndex, VkSemaphore renderFinishedSemaphore );

        private:
            void CreateSwapchain ( CEWindow * window, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
            void CreateImageViews ();
            void CreateRenderPass ();
            void CreateFramebuffers ();
            void CreateDepthResources ();
            void CleanupSwapchain ();

            // Helper methods
            VkSurfaceFormatKHR ChooseSwapSurfaceFormat ( const CEArray<VkSurfaceFormatKHR> & availableFormats );
            VkPresentModeKHR ChooseSwapPresentMode ( const CEArray<VkPresentModeKHR> & availablePresentModes );
            VkExtent2D ChooseSwapExtent ( CEWindow * window, const VkSurfaceCapabilitiesKHR & capabilities );

            CEVulkanContext * m_Context = nullptr;

            // Swapchain objects
            VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
            CEArray<VkImage> m_Images;
            CEArray<VkImageView> m_ImageViews;
            VkFormat m_ImageFormat;
            VkExtent2D m_Extent;

            // Render pass and framebuffers
            VkRenderPass m_RenderPass = VK_NULL_HANDLE;
            CEArray<VkFramebuffer> m_Framebuffers;

            // Depth resources
            VkImage m_DepthImage = VK_NULL_HANDLE;
            VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
            VkImageView m_DepthImageView = VK_NULL_HANDLE;
        };
    }