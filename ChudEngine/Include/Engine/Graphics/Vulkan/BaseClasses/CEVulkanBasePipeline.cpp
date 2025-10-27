// Runtime/Renderer/Vulkan/CEVulkanBasePipeline.cpp
#include "CEVulkanBasePipeline.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Utils/Logger.hpp"
#include <stdexcept>

namespace CE
    {
    CEVulkanBasePipeline::CEVulkanBasePipeline (
        CEVulkanContext * context,
        CEVulkanShaderManager * shaderManager,
        const PipelineConfig & config )
        : m_Context ( context )
        , m_ShaderManager ( shaderManager )
        , m_Config ( config )
        {
        CE_CORE_DEBUG ( "Base pipeline '{}' created", m_Config.Name );
        }

    CEVulkanBasePipeline::~CEVulkanBasePipeline ()
        {
        Shutdown ();
        }

    bool CEVulkanBasePipeline::Initialize ( VkRenderPass renderPass )
        {
        CE_CORE_DEBUG ( "Initializing base pipeline '{}'", m_Config.Name );

        // Загружаем шейдеры если пути установлены
        if (!m_VertexShaderPath.empty () && !SetVertexShader ( m_VertexShaderPath ))
            {
            CE_CORE_ERROR ( "Failed to set vertex shader: {}", m_VertexShaderPath );
            return false;
            }

        if (!m_FragmentShaderPath.empty () && !SetFragmentShader ( m_FragmentShaderPath ))
            {
            CE_CORE_ERROR ( "Failed to set fragment shader: {}", m_FragmentShaderPath );
            return false;
            }

            // Создаем компоненты пайплайна
        if (!CreateDescriptorSetLayout ()) return false;
        if (!CreatePipelineLayout ()) return false;
        if (!CreateGraphicsPipeline ( renderPass )) return false;

        CE_CORE_DEBUG ( "Base pipeline '{}' initialized successfully", m_Config.Name );
        return true;
        }

    void CEVulkanBasePipeline::Shutdown ()
        {
        auto device = m_Context->GetDevice ()->GetDevice();

        if (m_GraphicsPipeline != VK_NULL_HANDLE)
            {
            vkDestroyPipeline ( device, m_GraphicsPipeline, nullptr );
            m_GraphicsPipeline = VK_NULL_HANDLE;
            }

        if (m_PipelineLayout != VK_NULL_HANDLE)
            {
            vkDestroyPipelineLayout ( device, m_PipelineLayout, nullptr );
            m_PipelineLayout = VK_NULL_HANDLE;
            }

        if (m_DescriptorSetLayout != VK_NULL_HANDLE)
            {
            vkDestroyDescriptorSetLayout ( device, m_DescriptorSetLayout, nullptr );
            m_DescriptorSetLayout = VK_NULL_HANDLE;
            }

            // Шейдеры управляются ShaderManager'ом
        m_VertexShader.reset ();
        m_FragmentShader.reset ();

        CE_CORE_DEBUG ( "Base pipeline '{}' shutdown complete", m_Config.Name );
        }

    void CEVulkanBasePipeline::Bind ( VkCommandBuffer commandBuffer )
        {
        if (m_GraphicsPipeline != VK_NULL_HANDLE)
            {
            vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline );
            }
        }

    bool CEVulkanBasePipeline::SetVertexShader ( const std::string & shaderPath )
        {
        m_VertexShaderPath = shaderPath;
        m_VertexShader = m_ShaderManager->CompileAndLoadShader ( shaderPath, VK_SHADER_STAGE_VERTEX_BIT );
        return m_VertexShader != nullptr;
        }

    bool CEVulkanBasePipeline::SetFragmentShader ( const std::string & shaderPath )
        {
        m_FragmentShaderPath = shaderPath;
        m_FragmentShader = m_ShaderManager->CompileAndLoadShader ( shaderPath, VK_SHADER_STAGE_FRAGMENT_BIT );
        return m_FragmentShader != nullptr;
        }

    bool CEVulkanBasePipeline::ReloadShaders ()
        {
        CE_CORE_DEBUG ( "Reloading shaders for pipeline '{}'", m_Config.Name );

        // Сохраняем пути
        std::string vertPath = m_VertexShaderPath;
        std::string fragPath = m_FragmentShaderPath;

        // Сбрасываем шейдеры
        m_VertexShader.reset ();
        m_FragmentShader.reset ();

        // Перезагружаем
        bool success = true;
        if (!vertPath.empty ()) success = SetVertexShader ( vertPath ) && success;
        if (!fragPath.empty ()) success = SetFragmentShader ( fragPath ) && success;

        if (success)
            {
            CE_CORE_DEBUG ( "Shaders reloaded successfully for pipeline '{}'", m_Config.Name );
            }
        else
            {
            CE_CORE_ERROR ( "Failed to reload shaders for pipeline '{}'", m_Config.Name );
            }

        return success;
        }

    bool CEVulkanBasePipeline::CreateDescriptorSetLayout ()
        {
        VkDescriptorSetLayoutBinding uboLayoutBinding {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout ( m_Context->GetDevice ()->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create descriptor set layout for pipeline '{}'", m_Config.Name );
            return false;
            }

        return true;
        }

    bool CEVulkanBasePipeline::CreatePipelineLayout ()
        {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout ( m_Context->GetDevice ()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create pipeline layout for pipeline '{}'", m_Config.Name );
            return false;
            }

        return true;
        }

    bool CEVulkanBasePipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        if (!m_Context || renderPass == VK_NULL_HANDLE || m_PipelineLayout == VK_NULL_HANDLE)
            {
            CE_CORE_ERROR ( "Invalid parameters in CreateGraphicsPipeline for '{}'", m_Config.Name );
            return false;
            }

        if (!m_VertexShader || !m_FragmentShader)
            {
            CE_CORE_ERROR ( "Missing shaders for pipeline '{}'", m_Config.Name );
            return false;
            }

        try
            {
                // Настраиваем шейдерные стадии
            auto vertStageInfo = CreateShaderStageInfo ( m_VertexShader );
            auto fragStageInfo = CreateShaderStageInfo ( m_FragmentShader );
            VkPipelineShaderStageCreateInfo shaderStages [] = { vertStageInfo, fragStageInfo };

            // Vertex input
            auto bindDescs = CEMeshComponent::Vertex::GetBindingDescription ();
            auto atribDesc = CEMeshComponent::Vertex::GetAttributeDescriptions ();

            VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindDescs;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( atribDesc.size () );
            vertexInputInfo.pVertexAttributeDescriptions = atribDesc.data ();

            // Input assembly
            VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = m_Config.Topology;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            // Viewport state (dynamic)
            VkPipelineViewportStateCreateInfo viewportState {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            // Rasterizer
            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = m_Config.PolygonMode;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = m_Config.CullMode;
            rasterizer.frontFace = m_Config.FrontFace;
            rasterizer.depthBiasEnable = VK_FALSE;

            // Multisampling
            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            // Color blending
            VkPipelineColorBlendAttachmentState colorBlendAttachment {};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = m_Config.BlendEnable ? VK_TRUE : VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;

            // Dynamic state
            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
                };
            VkPipelineDynamicStateCreateInfo dynamicState {};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast< uint32_t >( dynamicStates.size () );
            dynamicState.pDynamicStates = dynamicStates.data ();

            // Depth stencil
            VkPipelineDepthStencilStateCreateInfo depthStencil {};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = m_Config.DepthTest ? VK_TRUE : VK_FALSE;
            depthStencil.depthWriteEnable = m_Config.DepthWrite ? VK_TRUE : VK_FALSE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;

            // Создаем пайплайн
            VkGraphicsPipelineCreateInfo pipelineInfo {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pDepthStencilState = &depthStencil;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = m_PipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            VkResult result = vkCreateGraphicsPipelines ( m_Context->GetDevice ()->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline);
            if (result != VK_SUCCESS)
                {
                CE_CORE_ERROR ( "Failed to create graphics pipeline for '{}': {}", m_Config.Name, static_cast< int >( result ) );
                return false;
                }

            CE_CORE_DEBUG ( "Graphics pipeline created successfully for '{}'", m_Config.Name );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Exception in CreateGraphicsPipeline for '{}': {}", m_Config.Name, e.what () );
                return false;
                }
        }

    VkPipelineShaderStageCreateInfo CEVulkanBasePipeline::CreateShaderStageInfo (
        std::shared_ptr<CEVulkanShaderManager::ShaderModule> shader )
        {
        VkPipelineShaderStageCreateInfo stageInfo {};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = shader->stage;
        stageInfo.module = shader->module;
        stageInfo.pName = "main";
        return stageInfo;
        }
    }