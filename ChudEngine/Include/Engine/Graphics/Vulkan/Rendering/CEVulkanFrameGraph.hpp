// Graphics/Vulkan/Rendering/CEVulkanFrameGraph.hpp
#pragma once
#include <vector>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanSwapchain;
    class CEVulkanCommandBuffer;
    class CEVulkanSync;

    struct FrameGraphNode
        {
        std::string name;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;
        std::vector<VkClearValue> clearValues;
        std::function<void ( VkCommandBuffer )> executeCallback;
        };

    class CEVulkanFrameGraph
        {
        public:
            CEVulkanFrameGraph ();
            ~CEVulkanFrameGraph ();

            bool Initialize ( CEVulkanContext * context, CEVulkanSwapchain * swapchain );
            void Shutdown ();

            void AddNode ( const FrameGraphNode & node );
            void RemoveNode ( const std::string & name );
            void ClearNodes ();

            bool BeginFrame ();
            bool EndFrame ();
            void ExecuteFrame ( VkCommandBuffer commandBuffer );

            void OnSwapchainRecreated ();

            uint32_t GetCurrentImageIndex () const { return m_CurrentImageIndex; }
            uint32_t GetCurrentFrameIndex () const { return m_CurrentFrame; }
            bool IsFrameInProgress () const { return m_FrameInProgress; }

        private:
            bool AcquireNextImage ();
            bool SubmitCommandBuffer ();

            CEVulkanContext * m_Context = nullptr;
            CEVulkanSwapchain * m_Swapchain = nullptr;
            CEVulkanCommandBuffer * m_CommandBuffer = nullptr;
            CEVulkanSync * m_SyncManager = nullptr;

            std::vector<FrameGraphNode> m_Nodes;
            uint32_t m_CurrentFrame = 0;
            uint32_t m_CurrentImageIndex = 0;
            bool m_FrameInProgress = false;
        };
    }