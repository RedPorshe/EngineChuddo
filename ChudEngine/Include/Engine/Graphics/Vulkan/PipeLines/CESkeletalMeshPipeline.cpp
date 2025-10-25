// Runtime/Renderer/Vulkan/Pipelines/CESkeletalMeshPipeline.cpp
#include "CESkeletalMeshPipeline.hpp"
#include "Core/Logger.h"

namespace CE
    {
    CESkeletalMeshPipeline::CESkeletalMeshPipeline ( CEVulkanContext * context )
        : CEVulkanBasePipeline ( context,
                                 PipelineConfig {
                                     "SkeletalMesh",
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

    bool CESkeletalMeshPipeline::Initialize ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::Initialize ( renderPass );
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