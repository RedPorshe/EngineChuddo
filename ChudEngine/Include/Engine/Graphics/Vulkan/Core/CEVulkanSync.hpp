// Graphics/Vulkan/Core/CEVulkanSync.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanSwapchain;

    class CEVulkanSync
        {
        public:
            CEVulkanSync ();
            ~CEVulkanSync ();

            bool Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight );
            void Shutdown ();

            VkResult AcquireNextImage ( CEVulkanSwapchain * swapchain, uint32_t * imageIndex );
            bool SubmitFrame ( VkCommandBuffer commandBuffer, CEVulkanSwapchain * swapchain, uint32_t imageIndex );

            VkSemaphore GetCurrentImageAvailableSemaphore () const { return m_ImageAvailableSemaphores[ m_CurrentFrame ]; }
            VkSemaphore GetCurrentRenderFinishedSemaphore () const { return m_RenderFinishedSemaphores[ m_CurrentFrame ]; }
            VkFence GetCurrentInFlightFence () const { return m_InFlightFences[ m_CurrentFrame ]; }
            uint32_t GetCurrentFrame () const { return m_CurrentFrame; }

            void AdvanceFrame ();

        private:
            void CreateSyncObjects ();

            CEVulkanContext * m_Context = nullptr;
            std::vector<VkSemaphore> m_ImageAvailableSemaphores;
            std::vector<VkSemaphore> m_RenderFinishedSemaphores;
            std::vector<VkFence> m_InFlightFences;
            uint32_t m_CurrentFrame = 0;
            uint32_t m_MaxFramesInFlight = 2;
        };
    }