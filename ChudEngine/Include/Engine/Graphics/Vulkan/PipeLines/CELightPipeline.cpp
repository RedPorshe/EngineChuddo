// Runtime/Renderer/Vulkan/Pipelines/CELightPipeline.cpp
#include "CELightPipeline.hpp"
#include "Core/Logger.h"

namespace CE
    {
    CELightPipeline::CELightPipeline ( CEVulkanContext * context )
        : CEVulkanBasePipeline ( context,
                                 PipelineConfig {
                                     "Light",
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     VK_POLYGON_MODE_FILL,
                                     VK_CULL_MODE_FRONT_BIT, // Light usually uses front face culling
                                     VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     true,   // depth test
                                     false,  // depth write - lights usually don't write depth
                                     true    // blend enable - lights use additive blending
                                 } )
        {
        }

    bool CELightPipeline::Initialize ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::Initialize ( renderPass );
        }

    bool CELightPipeline::CreateDescriptorSetLayout ()
        {
        return CEVulkanBasePipeline::CreateDescriptorSetLayout ();
        }

    bool CELightPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::CreateGraphicsPipeline ( renderPass );
        }

    void CELightPipeline::UpdateLightData ( uint32_t currentImage, const Math::Vector3 & position,
                                            const Math::Vector3 & color, float intensity )
        {
            // TODO: Implement light data update
        CE_CORE_DEBUG ( "Updating light data: position=({}, {}, {}), color=({}, {}, {}), intensity={}",
                        position.x, position.y, position.z, color.x, color.y, color.z, intensity );
        }
    }