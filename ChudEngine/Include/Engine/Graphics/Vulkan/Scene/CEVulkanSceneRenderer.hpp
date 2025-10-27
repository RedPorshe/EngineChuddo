// Graphics/Vulkan/Scene/CEVulkanSceneRenderer.hpp
#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace CE
    {
    class CEWorld;
    class CEVulkanRenderer;
    class CEVulkanCamera;
    class CEVulkanPipelineManager;
    class CEVulkanResourceManager;

    class CEVulkanSceneRenderer
        {
        public:
            CEVulkanSceneRenderer ( CEVulkanRenderer * renderer );
            ~CEVulkanSceneRenderer ();

            bool Initialize ();
            void Shutdown ();

            void SetWorld ( CEWorld * world ) { m_World = world; }
            void SetCamera ( CEVulkanCamera * camera ) { m_Camera = camera; }

            void Render ( VkCommandBuffer commandBuffer );
            void Update ( float deltaTime );

        private:
            void RenderStaticMeshes ( VkCommandBuffer commandBuffer );
            void RenderSkeletalMeshes ( VkCommandBuffer commandBuffer );
            void RenderLights ( VkCommandBuffer commandBuffer );
            void RenderDebug ( VkCommandBuffer commandBuffer );

            CEWorld * m_World = nullptr;
            CEVulkanRenderer * m_Renderer = nullptr;
            CEVulkanCamera * m_Camera = nullptr;

            bool m_Initialized = false;
        };
    }