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
            CEVulkanCommandBuffer ( CEVulkanContext * context );
            ~CEVulkanCommandBuffer ();

            bool Initialize ();
            void Shutdown ();

            VkCommandBuffer GetCommandBuffer () const { return CommandBuffer; }
            void BeginRecording ();
            void EndRecording ();
            void Reset ();
            bool IsReadyForRecording () const;
            void ValidateRecordingState () const;
        private:
            void CreateCommandPool ();
            void CreateCommandBuffer ();
            CEVulkanContext * Context = nullptr;
            VkCommandPool CommandPool = VK_NULL_HANDLE;
            VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
        };
    }