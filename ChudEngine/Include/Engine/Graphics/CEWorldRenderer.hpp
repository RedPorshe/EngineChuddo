// CEWorldRenderer.hpp
#pragma once

#include "Core/CEObject/CEWorld.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Graphics/Vulkan/Pipelines/CEStaticMeshPipeline.hpp"

// Вместо прямого включения CEVulkanRenderer используем forward declaration
namespace CE
    {
    class CEVulkanRenderer;
    class CEVulkanPipelineManager;

    class CEWorldRenderer
        {
        public:
            CEWorldRenderer ( CEVulkanRenderer * renderer );
            ~CEWorldRenderer ();

            void SetWorld ( CEWorld * world ) { m_World = world; }
            void Render ( VkCommandBuffer commandBuffer );

        private:
            std::vector<CEMeshComponent *> GatherMeshComponents ();
            void RenderMeshComponent ( CEMeshComponent * meshComponent,
                                       VkCommandBuffer commandBuffer,
                                       CEStaticMeshPipeline * pipeline );
            bool EnsureMeshBuffersCreated ( CEMeshComponent * meshComponent );

            CEWorld * m_World = nullptr;
            CEVulkanRenderer * m_Renderer = nullptr;
        };
    }