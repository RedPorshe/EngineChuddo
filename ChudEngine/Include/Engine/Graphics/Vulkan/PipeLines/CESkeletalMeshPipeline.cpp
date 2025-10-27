#include "Graphics/Vulkan/PipeLines/CESkeletalMeshPipeline.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CESkeletalMeshPipeline::CESkeletalMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : CEVulkanBasePipeline ( context, shaderManager, PipelineConfig { "SkeletalMeshPipeline" } )
        {
         // Устанавливаем специфичные для SkeletalMeshPipeline шейдеры
      //  SetVertexShader ( "Resources/Shaders/Vulkan/skeletal.vert" );
     //   SetFragmentShader ( "Resources/Shaders/Vulkan/skeletal.frag" );
        }

  

    bool CESkeletalMeshPipeline::CreateDescriptorSetLayout ()
        {
        return CEVulkanBasePipeline::CreateDescriptorSetLayout ();
        }

    bool CESkeletalMeshPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::CreateGraphicsPipeline ( renderPass );
        }

    void CESkeletalMeshPipeline::UpdateBoneMatrices ( uint32_t currentImage, const std::vector<Math::Matrix4> & boneMatrices )
        {
            // TODO: Implement bone matrices update
        CE_CORE_DEBUG ( "Updating {} bone matrices", boneMatrices.size () );
        }
    }