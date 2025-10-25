// Runtime/Renderer/Vulkan/Pipelines/CEStaticMeshPipeline.cpp
#include "CEStaticMeshPipeline.hpp"
#include "Core/Logger.h"

namespace CE
    {
    CEStaticMeshPipeline::CEStaticMeshPipeline ( CEVulkanContext * context )
        : CEVulkanBasePipeline ( context,
                                 PipelineConfig {
                                     "StaticMesh",
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     VK_POLYGON_MODE_FILL,
                                     VK_CULL_MODE_BACK_BIT,
                                     VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     true,  // depth test
                                     true,  // depth write
                                     false  // blend enable
                                 } )
        {
        }

    bool CEStaticMeshPipeline::Initialize ( VkRenderPass renderPass )
        {
            // Используем базовую реализацию
        return CEVulkanBasePipeline::Initialize ( renderPass );
        }

    bool CEStaticMeshPipeline::CreateDescriptorSetLayout ()
        {
            // Используем базовую реализацию
        return CEVulkanBasePipeline::CreateDescriptorSetLayout ();
        }

    bool CEStaticMeshPipeline::CreatePipelineLayout ()
        {
            // Используем базовую реализацию
        return CEVulkanBasePipeline::CreatePipelineLayout ();
        }

    bool CEStaticMeshPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
            // Используем базовую реализацию
        return CEVulkanBasePipeline::CreateGraphicsPipeline ( renderPass );
        }

    void CEStaticMeshPipeline::UpdatePerObjectUniforms ( uint32_t currentImage, uint32_t objectIndex,
                                                         const Math::Matrix4 & model, const Math::Vector4 & color )
        {
        CE_DEBUG ( "=== Updating Uniforms for Object {} ===", objectIndex );
       
        CE_DEBUG ( "Color: ({:.1f}, {:.1f}, {:.1f}, {:.1f})", color.x, color.y, color.z, color.w );

        // TODO: Реальная реализация обновления uniform буферов
        }

    void CEStaticMeshPipeline::BindMeshData ( VkCommandBuffer commandBuffer, VkBuffer vertexBuffer,
                                              VkBuffer indexBuffer, uint32_t indexCount )
        {
            // TODO: Implement mesh data binding
        CE_CORE_DEBUG ( "Binding mesh data with {} indices", indexCount );
        }
    }