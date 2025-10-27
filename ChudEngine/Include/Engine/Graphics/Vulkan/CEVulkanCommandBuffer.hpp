// Runtime/Renderer/Vulkan/CEVulkanCommandBuffer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "Core/Containers/CEArray.hpp"
#include "CEVulkanContext.hpp"

namespace CE
    {
    class CEVulkanCommandBuffer
        {
        public:
            CEVulkanCommandBuffer ();
            ~CEVulkanCommandBuffer ();

            bool Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight );
            void Shutdown ();

            // Command buffer operations
            VkCommandBuffer GetCurrent () const {
                return m_CommandBuffers[ m_CurrentFrame ];
                }
            void ResetCurrent ();
            void BeginRecording ();
            void EndRecording ();

            void AdvanceFrame () {
                m_CurrentFrame = ( m_CurrentFrame + 1 ) % m_CommandBuffers.Size ();
                }

            bool IsReadyForRecording () const;

        private:
            void CreateCommandPool ();
            void CreateCommandBuffers ();

            CEVulkanContext * m_Context = nullptr;
            VkCommandPool m_CommandPool = VK_NULL_HANDLE;
            CEArray<VkCommandBuffer> m_CommandBuffers;
            uint32_t m_CurrentFrame = 0;
        };
    }