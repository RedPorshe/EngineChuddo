// Graphics/Vulkan/Core/CEVulkanCommandBuffer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanCommandBuffer
        {
        public:
            CEVulkanCommandBuffer ();
            ~CEVulkanCommandBuffer ();

            bool Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight );
            void Shutdown ();

            VkCommandBuffer GetCurrent () const { return m_CommandBuffers[ m_CurrentFrame ]; }
            VkCommandBuffer GetByIndex ( uint32_t index ) const { return m_CommandBuffers[ index ]; }
            void ResetCurrent ();
            void BeginRecording ();
            void EndRecording ();
            void Submit ( VkQueue queue, VkSemaphore waitSemaphore = VK_NULL_HANDLE,
                          VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE );

            void AdvanceFrame () { m_CurrentFrame = ( m_CurrentFrame + 1 ) % m_CommandBuffers.size (); }
            bool IsReadyForRecording () const;
            uint32_t GetCurrentFrameIndex () const { return m_CurrentFrame; }

        private:
            void CreateCommandPool ();
            void CreateCommandBuffers ();

            CEVulkanContext * m_Context = nullptr;
            VkCommandPool m_CommandPool = VK_NULL_HANDLE;
            std::vector<VkCommandBuffer> m_CommandBuffers;
            uint32_t m_CurrentFrame = 0;
        };
    }