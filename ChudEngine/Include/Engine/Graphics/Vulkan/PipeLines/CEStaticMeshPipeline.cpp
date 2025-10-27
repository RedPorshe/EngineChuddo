#include "Graphics/Vulkan/Pipelines/CEStaticMeshPipeline.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEStaticMeshPipeline::CEStaticMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : CEVulkanBasePipeline ( context, shaderManager,
                                 PipelineConfig {
                                     "StaticMesh",
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     VK_POLYGON_MODE_FILL,
                                     VK_CULL_MODE_BACK_BIT,
                                     VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                     true,  // depth test
                                     true,  // depth write
                                     false, // blend enable
                                     VK_SAMPLE_COUNT_1_BIT
                                 } )
        {
            // Set default shader paths
        SetVertexShader ( "shaders/static_mesh.vert.spv" );
        SetFragmentShader ( "shaders/static_mesh.frag.spv" );
        }

    bool CEStaticMeshPipeline::CreateDescriptorSetLayout ()
        {
            // Static mesh pipeline uses a simple descriptor set layout
            // Typically for transformation matrices and material properties

        std::vector<VkDescriptorSetLayoutBinding> bindings;

        // Binding 0: Uniform buffer for scene data (view/projection matrices)
        VkDescriptorSetLayoutBinding sceneUBOBinding = {};
        sceneUBOBinding.binding = 0;
        sceneUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        sceneUBOBinding.descriptorCount = 1;
        sceneUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        sceneUBOBinding.pImmutableSamplers = nullptr;
        bindings.push_back ( sceneUBOBinding );

        // Binding 1: Uniform buffer for model data (per-object transforms)
        VkDescriptorSetLayoutBinding modelUBOBinding = {};
        modelUBOBinding.binding = 1;
        modelUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelUBOBinding.descriptorCount = 1;
        modelUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelUBOBinding.pImmutableSamplers = nullptr;
        bindings.push_back ( modelUBOBinding );

        // Binding 2: Combined image sampler for diffuse texture
        VkDescriptorSetLayoutBinding samplerBinding = {};
        samplerBinding.binding = 2;
        samplerBinding.descriptorCount = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings.push_back ( samplerBinding );

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

        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create static mesh descriptor set layout" );
            return false;
            }

        return true;
        }

    bool CEStaticMeshPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        return CEVulkanBasePipeline::CreateGraphicsPipeline ( renderPass );
        }
    }