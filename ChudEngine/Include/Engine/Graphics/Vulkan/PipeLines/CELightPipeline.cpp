// Runtime/Renderer/Vulkan/Pipelines/CELightPipeline.cpp
#include "CELightPipeline.hpp"
#include "Core/Logger.h"

namespace CE
    {
    CELightPipeline::CELightPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : CEVulkanBasePipeline ( context, shaderManager, PipelineConfig { "LightPipeline" } )
        {
         // Устанавливаем специфичные для LightPipeline шейдеры
      //  SetVertexShader ( "Resources/Shaders/Vulkan/light.vert" );
      //  SetFragmentShader ( "Resources/Shaders/Vulkan/light.frag" );
       
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