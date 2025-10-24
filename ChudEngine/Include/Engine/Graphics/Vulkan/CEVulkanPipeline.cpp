#include "CEVulkanPipeline.hpp"
#include "Core/Logger.h"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "ShaderCompiler.h"
#include "Math/Vector.hpp"
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <filesystem>

namespace CE
    {
    const int MAX_FRAMES_IN_FLIGHT = 2;

    CEVulkanPipeline::CEVulkanPipeline ( CEVulkanContext * context )
        : m_Context ( context )
        {
        CE_CORE_DEBUG ( "Vulkan pipeline created" );

        // Устанавливаем пути к шейдерам по умолчанию
        m_VertexShaderPath = "Resources/Shaders/Vulkan/triangle.vert";
        m_FragmentShaderPath = "Resources/Shaders/Vulkan/triangle.frag";
        }

    CEVulkanPipeline::~CEVulkanPipeline ()
        {
        Shutdown ();
        }

    bool CEVulkanPipeline::Initialize ( VkRenderPass renderPass )
        {
        try
            {
                // 1. Компилируем шейдеры
            if (!CompileShaders ())
                {
                CE_CORE_ERROR ( "Failed to compile shaders" );
                return false;
                }

                // 2. Создаем layout и pool
            if (!CreateDescriptorSetLayout ()) return false;
            if (!CreateDescriptorPool ()) return false;

            // 3. Создаем буферы ПЕРЕД дескрипторными сетами
            if (!CreateUniformBuffers ()) return false;
            if (!CreateVertexBuffer ()) return false;

            // 4. Создаем дескрипторные сеты (теперь uniform buffers существуют)
            if (!CreateDescriptorSets ()) return false;

            // 5. Создаем пайплайн
            if (!CreateGraphicsPipeline ( renderPass )) return false;

            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan pipeline: {}", e.what () );
                return false;
                }
        }

    void CEVulkanPipeline::Shutdown ()
        {
        auto device = m_Context->GetDevice ();

        // Уничтожаем шейдерные модули
        DestroyShaderModules ();

        if (GraphicsPipeline != VK_NULL_HANDLE)
            {
            vkDestroyPipeline ( device, GraphicsPipeline, nullptr );
            GraphicsPipeline = VK_NULL_HANDLE;
            }

        if (PipelineLayout != VK_NULL_HANDLE)
            {
            vkDestroyPipelineLayout ( device, PipelineLayout, nullptr );
            PipelineLayout = VK_NULL_HANDLE;
            }

        if (m_DescriptorSetLayout != VK_NULL_HANDLE)
            {
            vkDestroyDescriptorSetLayout ( device, m_DescriptorSetLayout, nullptr );
            m_DescriptorSetLayout = VK_NULL_HANDLE;
            }

        if (m_DescriptorPool != VK_NULL_HANDLE)
            {
            vkDestroyDescriptorPool ( device, m_DescriptorPool, nullptr );
            m_DescriptorPool = VK_NULL_HANDLE;
            }

            // Cleanup uniform buffers
        for (auto & buffer : m_UniformBuffers)
            {
            if (buffer)
                {
                buffer->Destroy ();
                }
            }
        m_UniformBuffers.clear ();
        m_UniformBuffersMapped.clear ();

        for (auto & buffer : m_DynamicUniformBuffers)
            {
            if (buffer)
                {
                buffer->Destroy ();
                }
            }
        m_DynamicUniformBuffers.clear ();
        m_DynamicUniformBuffersMapped.clear ();

        CE_CORE_DEBUG ( "Vulkan pipeline shutdown complete" );
        }

        // ============================ МЕТОДЫ ДЛЯ ШЕЙДЕРОВ ============================

    bool CEVulkanPipeline::SetVertexShader ( const std::string & shaderPath )
        {
        if (std::filesystem::exists ( shaderPath ))
            {
            m_VertexShaderPath = shaderPath;
            CE_CORE_DEBUG ( "Vertex shader path set to: {}", shaderPath );
            return true;
            }
        else
            {
            CE_CORE_ERROR ( "Vertex shader file not found: {}", shaderPath );
            return false;
            }
        }

    bool CEVulkanPipeline::SetFragmentShader ( const std::string & shaderPath )
        {
        if (std::filesystem::exists ( shaderPath ))
            {
            m_FragmentShaderPath = shaderPath;
            CE_CORE_DEBUG ( "Fragment shader path set to: {}", shaderPath );
            return true;
            }
        else
            {
            CE_CORE_ERROR ( "Fragment shader file not found: {}", shaderPath );
            return false;
            }
        }

    bool CEVulkanPipeline::CompileShaders ()
        {
        try
            {
            CE_CORE_DEBUG ( "Compiling shaders..." );

            // Компилируем вершинный шейдер
            m_VertexShaderCode = CompileAndLoadShader ( m_VertexShaderPath );

            if (m_VertexShaderCode.empty ())
                {
                CE_CORE_ERROR ( "Failed to compile vertex shader: {}", m_VertexShaderPath );
                return false;
                }

                // Компилируем фрагментный шейдер
            m_FragmentShaderCode = CompileAndLoadShader ( m_FragmentShaderPath );

            if (m_FragmentShaderCode.empty ())
                {
                CE_CORE_ERROR ( "Failed to compile fragment shader: {}", m_FragmentShaderPath );
                return false;
                }

            CE_CORE_DEBUG ( "Shaders compiled successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Exception during shader compilation: {}", e.what () );
                return false;
                }
        }

    bool CEVulkanPipeline::ReloadShaders ()
        {
        CE_CORE_DEBUG ( "Reloading shaders..." );

        // Очищаем текущие шейдерные модули
        DestroyShaderModules ();

        // Очищаем текущие коды шейдеров
        m_VertexShaderCode.clear ();
        m_FragmentShaderCode.clear ();

        // Перекомпилируем шейдеры
        if (!CompileShaders ())
            {
            CE_CORE_ERROR ( "Failed to recompile shaders" );
            return false;
            }

        CE_CORE_DEBUG ( "Shaders reloaded successfully" );
        return true;
        }

    std::vector<uint32_t> CEVulkanPipeline::LoadShader ( const std::string & filename )
        {
        std::vector<uint32_t> code;

        std::ifstream file ( filename, std::ios::binary | std::ios::ate );
        if (!file.is_open ())
            {
            CE_CORE_ERROR ( "Failed to open shader file: {}", filename );
            return code;
            }

        size_t fileSize = file.tellg ();
        file.seekg ( 0 );
        code.resize ( fileSize / sizeof ( uint32_t ) );
        file.read ( reinterpret_cast< char * >( code.data () ), fileSize );
        file.close ();

        CE_CORE_DEBUG ( "Loaded shader: {} ({} bytes)", filename, fileSize );
        return code;
        }

    std::vector<uint32_t> CEVulkanPipeline::CompileAndLoadShader ( const std::string & sourcePath )
        {
        if (!std::filesystem::exists ( sourcePath ))
            {
            CE_CORE_ERROR ( "Shader source file not found: {}", sourcePath );
            return std::vector<uint32_t> ();
            }

            // Определяем выходной путь
        std::string outputPath = sourcePath + ".spv";

        // Проверяем, нужно ли перекомпилировать
        bool needsCompilation = true;

        if (std::filesystem::exists ( outputPath ))
            {
            auto sourceTime = std::filesystem::last_write_time ( sourcePath );
            auto outputTime = std::filesystem::last_write_time ( outputPath );

            if (sourceTime <= outputTime)
                {
                needsCompilation = false;
                CE_CORE_DEBUG ( "Using cached shader: {}", outputPath );
                }
            else
                {
                CE_CORE_DEBUG ( "Source shader is newer, recompiling: {}", sourcePath );
                }
            }

            // Компилируем если нужно
        if (needsCompilation)
            {
            if (!CompileShaderFile ( sourcePath, outputPath ))
                {
                CE_CORE_ERROR ( "Failed to compile shader: {}", sourcePath );
                return std::vector<uint32_t> ();
                }
            }

            // Загружаем скомпилированный шейдер
        return LoadShader ( outputPath );
        }

    bool CEVulkanPipeline::CompileShaderFile ( const std::string & sourcePath, const std::string & outputPath )
        {
        CE_CORE_DEBUG ( "Compiling shader: {} -> {}", sourcePath, outputPath );

        // Используем наш ShaderCompiler
        CE::ShaderCompiler compiler;
        compiler.SetOptimizationLevel ( 0 );
        compiler.SetGenerateDebugInfo ( true );

        auto result = compiler.CompileShader ( sourcePath, outputPath );

        if (result.success)
            {
            CE_CORE_DEBUG ( "Shader compiled successfully: {}", sourcePath );
            return true;
            }
        else
            {
            CE_CORE_ERROR ( "Shader compilation failed: {} - {}", sourcePath, result.message );
            return false;
            }
        }

    void CEVulkanPipeline::DestroyShaderModules ()
        {
        auto device = m_Context->GetDevice ();

        if (m_VertexShaderModule != VK_NULL_HANDLE)
            {
            vkDestroyShaderModule ( device, m_VertexShaderModule, nullptr );
            m_VertexShaderModule = VK_NULL_HANDLE;
            }

        if (m_FragmentShaderModule != VK_NULL_HANDLE)
            {
            vkDestroyShaderModule ( device, m_FragmentShaderModule, nullptr );
            m_FragmentShaderModule = VK_NULL_HANDLE;
            }
        }

        // ============================ ГРАФИЧЕСКИЙ ПАЙПЛАЙН ============================

    bool CEVulkanPipeline::CreateGraphicsPipeline ( VkRenderPass renderPass ) {
        auto device = m_Context->GetDevice ();
        CE_CORE_DEBUG ( "=== Creating graphics pipeline ===" );

        try
            {
                // Проверяем что шейдеры загружены
            if (m_VertexShaderCode.empty () || m_FragmentShaderCode.empty ())
                {
                CE_CORE_ERROR ( "Shader codes are empty! Vertex: {}, Fragment: {}",
                                m_VertexShaderCode.size (), m_FragmentShaderCode.size () );
                return false;
                }

                // Создаем шейдерные модули
            m_VertexShaderModule = CreateShaderModule ( m_VertexShaderCode );
            m_FragmentShaderModule = CreateShaderModule ( m_FragmentShaderCode );

            VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = m_VertexShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = m_FragmentShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages [] = { vertShaderStageInfo, fragShaderStageInfo };

            auto bindingDescription = Vertex::GetBindingDescription ();
            auto attributeDescriptions = Vertex::GetAttributeDescriptions ();

            VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size () );
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data ();

            // Input assembly
            VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            // Viewport state
            VkPipelineViewportStateCreateInfo viewportState {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            // Rasterizer
            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            // Multisampling
            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            // Color blending
            VkPipelineColorBlendAttachmentState colorBlendAttachment {};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

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
            depthStencil.depthTestEnable = VK_FALSE;
            depthStencil.depthWriteEnable = VK_FALSE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;

            // Pipeline layout с descriptor set
            VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
            pipelineLayoutInfo.pushConstantRangeCount = 0;

            CE_CORE_DEBUG ( "Creating pipeline layout..." );
            VkResult layoutResult = vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &PipelineLayout );
            if (layoutResult != VK_SUCCESS)
                {
                CE_CORE_ERROR ( "Failed to create pipeline layout: {}", static_cast< int >( layoutResult ) );
                return false;
                }

                // Create graphics pipeline
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
            pipelineInfo.layout = PipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            CE_CORE_DEBUG ( "Creating graphics pipeline..." );
            VkResult pipelineResult = vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GraphicsPipeline );

            if (pipelineResult != VK_SUCCESS)
                {
                CE_CORE_ERROR ( "Failed to create graphics pipeline: {}", static_cast< int >( pipelineResult ) );
                return false;
                }

            CE_CORE_DEBUG ( "Graphics pipeline created successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Exception in CreateGraphicsPipeline: {}", e.what () );
                return false;
                }
        }

    VkShaderModule CEVulkanPipeline::CreateShaderModule ( const std::vector<uint32_t> & code )
        {
        auto device = m_Context->GetDevice ();

        VkShaderModuleCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size () * sizeof ( uint32_t );
        createInfo.pCode = code.data ();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule ( device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create shader module!" );
            }

        return shaderModule;
        }

        // ============================ ДЕСКРИПТОРЫ И БУФЕРЫ ============================

    bool CEVulkanPipeline::CreateDescriptorSetLayout () {
        VkDescriptorSetLayoutBinding uboLayoutBinding {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout ( m_Context->GetDevice (), &layoutInfo, nullptr, &m_DescriptorSetLayout ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create descriptor set layout!" );
            return false;
            }

        CE_CORE_DEBUG ( "Descriptor set layout created (dynamic)" );
        return true;
        }

    bool CEVulkanPipeline::CreateUniformBuffers () {
        auto device = m_Context->GetDevice ();

        // Получаем требования к выравниванию
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties ( m_Context->GetPhysicalDevice (), &properties );
        m_UniformBufferAlignment = properties.limits.minUniformBufferOffsetAlignment;

        // Вычисляем динамическое выравнивание
        size_t uboSize = sizeof ( UniformBufferObject );
        m_DynamicAlignment = uboSize;

        if (m_UniformBufferAlignment > 0)
            {
            m_DynamicAlignment = ( uboSize + m_UniformBufferAlignment - 1 ) & ~( m_UniformBufferAlignment - 1 );
            }

            // ДОПОЛНИТЕЛЬНАЯ ПРОВЕРКА
        if (uboSize > m_DynamicAlignment)
            {
            CE_CORE_ERROR ( "UniformBufferObject size {} exceeds dynamic alignment {}", uboSize, m_DynamicAlignment );
            return false;
            }

        CE_CORE_DEBUG ( "UBO size: {}, Uniform buffer alignment: {}, Dynamic alignment: {}",
                        uboSize, m_UniformBufferAlignment, m_DynamicAlignment );

                    // Создаем буферы для нескольких объектов
        VkDeviceSize bufferSize = m_DynamicAlignment * MAX_OBJECTS;

        m_DynamicUniformBuffers.resize ( MAX_FRAMES_IN_FLIGHT );
        m_DynamicUniformBuffersMapped.resize ( MAX_FRAMES_IN_FLIGHT );

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
            m_DynamicUniformBuffers[ i ] = std::make_unique<CEVulkanBuffer> ();

            bool success = m_DynamicUniformBuffers[ i ]->Create (
                m_Context,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

            if (!success)
                {
                CE_CORE_ERROR ( "Failed to create dynamic uniform buffer for frame {}", i );
                return false;
                }

            m_DynamicUniformBuffersMapped[ i ] = m_DynamicUniformBuffers[ i ]->GetMappedData ();
            CE_CORE_DEBUG ( "Created dynamic uniform buffer for frame {} ({} bytes, {} objects, {} bytes per object)",
                            i, bufferSize, MAX_OBJECTS, m_DynamicAlignment );
            }

        return true;
        }

    void CEVulkanPipeline::UpdateUniforms ( uint32_t currentImage, const Math::Matrix4 & modelMatrix ) {
        if (currentImage >= m_UniformBuffersMapped.size ())
            {
            CE_CORE_ERROR ( "Invalid currentImage index: {}", currentImage );
            return;
            }

        if (!m_UniformBuffersMapped[ currentImage ])
            {
            CE_CORE_ERROR ( "Uniform buffer for image {} is not mapped!", currentImage );
            return;
            }

        UniformBufferObject ubo {};
        ubo.model = modelMatrix;
        ubo.view = m_ViewMatrix;
        ubo.proj = m_ProjectionMatrix;

        memcpy ( m_UniformBuffersMapped[ currentImage ], &ubo, sizeof ( ubo ) );
        }

    VkBuffer CEVulkanPipeline::GetUniformBuffer ( uint32_t frameIndex ) const
        {
        if (frameIndex < m_UniformBuffers.size ())
            {
            return m_UniformBuffers[ frameIndex ]->GetBuffer ();
            }
        return VK_NULL_HANDLE;
        }

    void CEVulkanPipeline::UpdateDynamicUniforms ( uint32_t currentImage, uint32_t objectIndex, const Math::Matrix4 & modelMatrix )
        {
        if (currentImage >= m_DynamicUniformBuffersMapped.size ())
            {
            CE_CORE_ERROR ( "Invalid currentImage index: {} (max: {})", currentImage, m_DynamicUniformBuffersMapped.size () - 1 );
            return;
            }

        if (!m_DynamicUniformBuffersMapped[ currentImage ])
            {
            CE_CORE_ERROR ( "Dynamic uniform buffer for image {} is not mapped!", currentImage );
            return;
            }

        if (objectIndex >= MAX_OBJECTS)
            {
            CE_CORE_ERROR ( "Object index {} exceeds maximum objects ({})", objectIndex, MAX_OBJECTS );
            return;
            }

        UniformBufferObject ubo {};
        ubo.model = modelMatrix;
        ubo.view = m_ViewMatrix;
        ubo.proj = m_ProjectionMatrix;

        // ПРОВЕРКА ВЫРАВНИВАНИЯ
        size_t offset = objectIndex * m_DynamicAlignment;
        if (offset + sizeof ( UniformBufferObject ) > m_DynamicUniformBuffers[ currentImage ]->GetSize ())
            {
            CE_CORE_ERROR ( "Buffer overflow: offset {} + ubo size {} > buffer size {}",
                            offset, sizeof ( UniformBufferObject ), m_DynamicUniformBuffers[ currentImage ]->GetSize () );
            return;
            }

            // Копируем данные в правильное смещение
        char * data = static_cast< char * >( m_DynamicUniformBuffersMapped[ currentImage ] );
        data += offset;
        memcpy ( data, &ubo, sizeof ( ubo ) );

        CE_DEBUG ( "Updated dynamic uniforms for frame {}, object {} at offset {} (alignment: {})",
                   currentImage, objectIndex, offset, m_DynamicAlignment );
        }

    VkDescriptorBufferInfo CEVulkanPipeline::GetDynamicUniformBufferInfo ( uint32_t frameIndex, uint32_t objectIndex ) const
        {
        VkDescriptorBufferInfo bufferInfo {};

        if (frameIndex < m_DynamicUniformBuffers.size () && objectIndex < MAX_OBJECTS)
            {
            bufferInfo.buffer = m_DynamicUniformBuffers[ frameIndex ]->GetBuffer ();
            bufferInfo.offset = objectIndex * m_DynamicAlignment;
            bufferInfo.range = sizeof ( UniformBufferObject );
            }
        else
            {
            CE_CORE_ERROR ( "Invalid frame index {} or object index {}", frameIndex, objectIndex );
            }

        return bufferInfo;
        }

    bool CEVulkanPipeline::CreateVertexBuffer ()
        {
        std::vector<Vertex> vertices = {
            {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
            };

        VkDeviceSize bufferSize = sizeof ( Vertex ) * vertices.size ();

        m_VertexBuffer = std::make_unique<CEVulkanBuffer> ();
        bool success = m_VertexBuffer->Create (
            m_Context,
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        if (success)
            {
            m_VertexBuffer->UploadData ( vertices.data (), bufferSize );
            CE_CORE_DEBUG ( "Vertex buffer created with {} vertices", vertices.size () );
            }
        else
            {
            CE_CORE_ERROR ( "Failed to create vertex buffer" );
            }

        return success;
        }

    void CEVulkanPipeline::BindVertexBuffer ( VkCommandBuffer commandBuffer )
        {
        if (!m_VertexBuffer || !m_VertexBuffer->IsValid ())
            return;

        VkBuffer vertexBuffers [] = { m_VertexBuffer->GetBuffer () };
        VkDeviceSize offsets [] = { 0 };
        vkCmdBindVertexBuffers ( commandBuffer, 0, 1, vertexBuffers, offsets );
        }

    bool CEVulkanPipeline::CreateDescriptorPool () {
        VkDescriptorPoolSize poolSize {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

        VkDescriptorPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

        if (vkCreateDescriptorPool ( m_Context->GetDevice (), &poolInfo, nullptr, &m_DescriptorPool ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create descriptor pool!" );
            return false;
            }
        return true;
        }

    bool CEVulkanPipeline::CreateDescriptorSets () {
        std::vector<VkDescriptorSetLayout> layouts ( MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout );

        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = layouts.data ();

        m_DescriptorSets.resize ( MAX_FRAMES_IN_FLIGHT );
        if (vkAllocateDescriptorSets ( m_Context->GetDevice (), &allocInfo, m_DescriptorSets.data () ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to allocate descriptor sets!" );
            return false;
            }

            // Обновляем descriptor sets для динамических буферов
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
            VkDescriptorBufferInfo bufferInfo {};
            bufferInfo.buffer = m_DynamicUniformBuffers[ i ]->GetBuffer ();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof ( UniformBufferObject );

            VkWriteDescriptorSet descriptorWrite {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSets[ i ];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;
            descriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets ( m_Context->GetDevice (), 1, &descriptorWrite, 0, nullptr );
            }

        CE_CORE_DEBUG ( "Dynamic descriptor sets created successfully" );
        return true;
        }

        // ============================ СТАРЫЕ МЕТОДЫ ДЛЯ СОВМЕСТИМОСТИ ============================

    std::vector<uint32_t> CEVulkanPipeline::CompileVertexShader () {
        return CompileAndLoadShader ( m_VertexShaderPath );
        }

    std::vector<uint32_t> CEVulkanPipeline::CompileFragmentShader () {
        return CompileAndLoadShader ( m_FragmentShaderPath );
        }
    }