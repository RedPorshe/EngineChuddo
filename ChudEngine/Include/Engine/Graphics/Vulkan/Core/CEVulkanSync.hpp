#pragma once
#include <vulkan/vulkan.h>
#include "Core/Containers/CEArray.hpp"

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanSwapchain;
    class CEVulkanCommandBuffer;

    class CEVulkanSync
        {
        public:
            CEVulkanSync ();
            ~CEVulkanSync ();

            bool Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight );
            void Shutdown ();

            // Frame synchronization
            VkResult AcquireNextImage ( CEVulkanSwapchain * swapchain, uint32_t * imageIndex );
            bool SubmitFrame ( CEVulkanCommandBuffer * commandBuffer, CEVulkanSwapchain * swapchain, uint32_t imageIndex );

            // Getters
            VkSemaphore GetCurrentImageAvailableSemaphore () const {
                return m_ImageAvailableSemaphores[ m_CurrentFrame ];
                }
            VkSemaphore GetCurrentRenderFinishedSemaphore () const {
                return m_RenderFinishedSemaphores[ m_CurrentFrame ];
                }
            VkFence GetCurrentInFlightFence () const {
                return m_InFlightFences[ m_CurrentFrame ];
                }
            uint32_t GetCurrentFrame () const { return m_CurrentFrame; }

            void AdvanceFrame ();

        private:
            void CreateSyncObjects ();

            CEVulkanContext * m_Context = nullptr;
            CEArray<VkSemaphore> m_ImageAvailableSemaphores;
            CEArray<VkSemaphore> m_RenderFinishedSemaphores;
            CEArray<VkFence> m_InFlightFences;
            uint32_t m_CurrentFrame = 0;
            uint32_t m_MaxFramesInFlight = 2;
        };
    }