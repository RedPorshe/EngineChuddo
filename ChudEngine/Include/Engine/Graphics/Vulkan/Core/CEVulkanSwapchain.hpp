// Graphics/Vulkan/Core/CEVulkanSwapchain.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace CE
    {
    class CEWindow;
    class CEVulkanContext;

    class CEVulkanSwapchain
        {
        public:
            CEVulkanSwapchain ();
            ~CEVulkanSwapchain ();

            bool Initialize ( CEVulkanContext * context, CEWindow * window, CEVulkanSwapchain * oldSwapchain = nullptr );
            void Shutdown ();

            VkSwapchainKHR GetSwapchain () const { return m_Swapchain; }
            VkRenderPass GetRenderPass () const { return m_RenderPass; }
            VkFormat GetImageFormat () const { return m_ImageFormat; }
            VkExtent2D GetExtent () const { return m_Extent; }
            const std::vector<VkImage> & GetImages () const { return m_Images; }
            const std::vector<VkImageView> & GetImageViews () const { return m_ImageViews; }
            const std::vector<VkFramebuffer> & GetFramebuffers () const { return m_Framebuffers; }
            VkFramebuffer GetFramebuffer ( uint32_t index ) const;
            uint32_t GetImageCount () const { return static_cast< uint32_t >( m_Images.size () ); }

            VkResult AcquireNextImage ( VkSemaphore imageAvailableSemaphore, uint32_t * imageIndex );
            VkResult Present ( uint32_t imageIndex, VkSemaphore renderFinishedSemaphore );

        private:
            void CreateSwapchain ( CEWindow * window, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
            void CreateImageViews ();
            void CreateRenderPass ();
            void CreateFramebuffers ();
            void CreateDepthResources ();
            void CreateMultisampleResources ();
            void CleanupSwapchain ();

            VkSurfaceFormatKHR ChooseSwapSurfaceFormat ( const std::vector<VkSurfaceFormatKHR> & availableFormats );
            VkPresentModeKHR ChooseSwapPresentMode ( const std::vector<VkPresentModeKHR> & availablePresentModes );
            VkExtent2D ChooseSwapExtent ( CEWindow * window, const VkSurfaceCapabilitiesKHR & capabilities );

            CEVulkanContext * m_Context = nullptr;
            CEWindow * m_Window = nullptr;

            VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
            std::vector<VkImage> m_Images;
            std::vector<VkImageView> m_ImageViews;
            VkFormat m_ImageFormat;
            VkExtent2D m_Extent;

            VkRenderPass m_RenderPass = VK_NULL_HANDLE;
            std::vector<VkFramebuffer> m_Framebuffers;

            VkImage m_DepthImage = VK_NULL_HANDLE;
            VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
            VkImageView m_DepthImageView = VK_NULL_HANDLE;

            VkImage m_ColorImage = VK_NULL_HANDLE;
            VkDeviceMemory m_ColorImageMemory = VK_NULL_HANDLE;
            VkImageView m_ColorImageView = VK_NULL_HANDLE;
        };
    }