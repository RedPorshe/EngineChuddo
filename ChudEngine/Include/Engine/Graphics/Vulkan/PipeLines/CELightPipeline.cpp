#include "Graphics/Vulkan/Pipelines/CELightPipeline.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CELightPipeline::CELightPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : CEVulkanBasePipeline ( context, shaderManager,
                                 PipelineConfig {
                                     "Light",
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     VK_POLYGON_MODE_FILL,
                                     VK_CULL_MODE_NONE,  // No culling for lights (or back face for spheres)
                                     VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     true,   // depth test
                                     false,  // depth write (lights don't write depth)
                                     true,   // blend enable (additive blending for multiple lights)
                                     VK_SAMPLE_COUNT_1_BIT
                                 } )
        {
        SetVertexShader ( "shaders/light.vert.spv" );
        SetFragmentShader ( "shaders/light.frag.spv" );
        }

    void CELightPipeline::UpdateLightData ( uint32_t currentImage, const Math::Vector3 & position,
                                            const Math::Vector3 & color, float intensity )
        {
            // This would update uniform buffers with light data
            // Implementation depends on your buffer management strategy
        CE_CORE_DEBUG ( "Updating light data: pos=({}, {}, {}), color=({}, {}, {}), intensity={}",
                        position.x, position.y, position.z,
                        color.x, color.y, color.z, intensity );
        }

    bool CELightPipeline::CreateDescriptorSetLayout ()
        {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        // Binding 0: Light uniform buffer
        VkDescriptorSetLayoutBinding lightUBOBinding = {};
        lightUBOBinding.binding = 0;
        lightUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightUBOBinding.descriptorCount = 1;
        lightUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings.push_back ( lightUBOBinding );

        // Binding 1: Scene uniform buffer (for view/projection matrices)
        VkDescriptorSetLayoutBinding sceneUBOBinding = {};
        sceneUBOBinding.binding = 1;
        sceneUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        sceneUBOBinding.descriptorCount = 1;
        sceneUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        bindings.push_back ( sceneUBOBinding );

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size () );
        layoutInfo.pBindings = bindings.data ();

        VkResult result = vkCreateDescriptorSetLayout (
            m_Context->GetDevice ()->GetDevice (),
            &layoutInfo,
            nullptr,
            &m_DescriptorSetLayout
        );

        return result == VK_SUCCESS;
        }

    bool CELightPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::CreateGraphicsPipeline ( renderPass );
        }
    }