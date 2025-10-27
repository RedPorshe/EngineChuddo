#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanBasePipeline::CEVulkanBasePipeline ( CEVulkanContext * context,
                                                 CEVulkanShaderManager * shaderManager,
                                                 const PipelineConfig & config )
        : m_Context ( context )
        , m_ShaderManager ( shaderManager )
        , m_Config ( config )
        , m_GraphicsPipeline ( VK_NULL_HANDLE )
        , m_PipelineLayout ( VK_NULL_HANDLE )
        , m_DescriptorSetLayout ( VK_NULL_HANDLE )
        {
        }

    CEVulkanBasePipeline::~CEVulkanBasePipeline ()
        {
        Shutdown ();
        }

    bool CEVulkanBasePipeline::Initialize ( VkRenderPass renderPass )
        {
        if (m_GraphicsPipeline != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Pipeline already initialized: {}", m_Config.Name );
            return true;
            }

        try
            {
            if (!CreateDescriptorSetLayout ())
                {
                throw std::runtime_error ( "Failed to create descriptor set layout" );
                }

            if (!CreatePipelineLayout ())
                {
                throw std::runtime_error ( "Failed to create pipeline layout" );
                }

            if (!CreateGraphicsPipeline ( renderPass ))
                {
                throw std::runtime_error ( "Failed to create graphics pipeline" );
                }

            CE_CORE_DEBUG ( "Pipeline initialized successfully: {}", m_Config.Name );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize pipeline {}: {}", m_Config.Name, e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanBasePipeline::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

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
            }

        m_VertexShader.reset ();
        m_FragmentShader.reset ();

        CE_CORE_DEBUG ( "Pipeline shut down: {}", m_Config.Name );
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
        m_VertexShader = m_ShaderManager->LoadShader ( shaderPath, VK_SHADER_STAGE_VERTEX_BIT );
        return m_VertexShader != nullptr;
        }

    bool CEVulkanBasePipeline::SetFragmentShader ( const std::string & shaderPath )
        {
        m_FragmentShaderPath = shaderPath;
        m_FragmentShader = m_ShaderManager->LoadShader ( shaderPath, VK_SHADER_STAGE_FRAGMENT_BIT );
        return m_FragmentShader != nullptr;
        }

    bool CEVulkanBasePipeline::ReloadShaders ()
        {
        CE_CORE_DEBUG ( "Reloading shaders for pipeline: {}", m_Config.Name );

        bool success = true;

        if (!m_VertexShaderPath.empty ())
            {
            if (!SetVertexShader ( m_VertexShaderPath ))
                {
                CE_CORE_ERROR ( "Failed to reload vertex shader: {}", m_VertexShaderPath );
                success = false;
                }
            }

        if (!m_FragmentShaderPath.empty ())
            {
            if (!SetFragmentShader ( m_FragmentShaderPath ))
                {
                CE_CORE_ERROR ( "Failed to reload fragment shader: {}", m_FragmentShaderPath );
                success = false;
                }
            }

        return success;
        }

    bool CEVulkanBasePipeline::CreateDescriptorSetLayout ()
        {
            // Base implementation - empty descriptor set layout
            // Derived classes should override this for specific layouts

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

    bool CEVulkanBasePipeline::CreatePipelineLayout ()
        {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;

        // Push constant range for matrices
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof ( MatrixPushConstants );

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        VkResult result = vkCreatePipelineLayout (
            m_Context->GetDevice ()->GetDevice (),
            &pipelineLayoutInfo,
            nullptr,
            &m_PipelineLayout
        );

        return result == VK_SUCCESS;
        }

    bool CEVulkanBasePipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
        {
        if (!m_VertexShader || !m_FragmentShader)
            {
            CE_CORE_ERROR ( "Missing shaders for pipeline creation: {}", m_Config.Name );
            return false;
            }

            // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            CreateShaderStageInfo ( m_VertexShader ),
            CreateShaderStageInfo ( m_FragmentShader )
            };

            // Vertex input
        auto bindingDescription = Vertex::GetBindingDescription ();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions ();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size () );
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data ();

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = m_Config.Topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Viewport and scissor
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 1.0f;  // Will be set dynamically
        viewport.height = 1.0f; // Will be set dynamically
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { 1, 1 }; // Will be set dynamically

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = m_Config.PolygonMode;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = m_Config.CullMode;
        rasterizer.frontFace = m_Config.FrontFace;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = m_Config.MsaaSamples;

        // Depth stencil
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = m_Config.DepthTest ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = m_Config.DepthWrite ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // Color blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = m_Config.BlendEnable ? VK_TRUE : VK_FALSE;
        if (m_Config.BlendEnable)
            {
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
            }

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[ 0 ] = 0.0f;
        colorBlending.blendConstants[ 1 ] = 0.0f;
        colorBlending.blendConstants[ 2 ] = 0.0f;
        colorBlending.blendConstants[ 3 ] = 0.0f;

        // Dynamic state
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
            };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast< uint32_t >( dynamicStates.size () );
        dynamicState.pDynamicStates = dynamicStates.data ();

        // Pipeline creation
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast< uint32_t >( shaderStages.size () );
        pipelineInfo.pStages = shaderStages.data ();
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
        pipelineInfo.basePipelineIndex = -1;

        VkResult result = vkCreateGraphicsPipelines (
            m_Context->GetDevice ()->GetDevice (),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &m_GraphicsPipeline
        );

        return result == VK_SUCCESS;
        }

    VkPipelineShaderStageCreateInfo CEVulkanBasePipeline::CreateShaderStageInfo (
        std::shared_ptr<CEVulkanShaderManager::ShaderModule> shader )
        {
        VkPipelineShaderStageCreateInfo stageInfo = {};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = shader->stage;
        stageInfo.module = shader->module;
        stageInfo.pName = shader->entryPoint.c_str ();
        return stageInfo;
        }
    }