#include "Graphics/Vulkan/Pipelines/CEDebugPipeline.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEDebugPipeline::CEDebugPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : CEVulkanBasePipeline ( context, shaderManager,
                                 PipelineConfig {
                                     "Debug",
                                     VK_PRIMITIVE_TOPOLOGY_LINE_LIST,  // Lines for debug rendering
                                     VK_POLYGON_MODE_FILL,
                                     VK_CULL_MODE_NONE,  // No culling for debug lines
                                     VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     true,   // depth test
                                     false,  // depth write (debug lines often drawn on top)
                                     false,  // blend enable
                                     VK_SAMPLE_COUNT_1_BIT
                                 } )
        {
        SetVertexShader ( "shaders/debug.vert.spv" );
        SetFragmentShader ( "shaders/debug.frag.spv" );
        }

    bool CEDebugPipeline::CreateDescriptorSetLayout ()
        {
            // Debug pipeline uses push constants for simple data
            // No descriptor sets needed for basic debug rendering

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 0;
        layoutInfo.pBindings = nullptr;

        VkResult result = vkCreateDescriptorSetLayout (
            m_Context->GetDevice ()->GetDevice (),
            &layoutInfo,
            nullptr,
            &m_DescriptorSetLayout
        );

        return result == VK_SUCCESS;
        }

    bool CEDebugPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::CreateGraphicsPipeline ( renderPass );
        }
    }