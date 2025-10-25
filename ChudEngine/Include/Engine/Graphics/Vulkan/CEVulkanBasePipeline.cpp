// Runtime/Renderer/Vulkan/CEVulkanBasePipeline.cpp
#include "CEVulkanBasePipeline.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Core/Logger.h"
#include "ShaderCompiler.h"
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace CE
	{

		// Вспомогательная функция для чтения файла
	static std::vector<char> ReadFile ( const std::string & filename )
		{
		std::ifstream file ( filename, std::ios::ate | std::ios::binary );

		if (!file.is_open ())
			{
			CE_CORE_ERROR ( "Failed to open file: {}", filename );
			throw std::runtime_error ( "Failed to open file: " + filename );
			}

		size_t fileSize = ( size_t ) file.tellg ();
		std::vector<char> buffer ( fileSize );

		file.seekg ( 0 );
		file.read ( buffer.data (), fileSize );

		if (!file)
			{
			CE_CORE_ERROR ( "Failed to read file: {}", filename );
			file.close ();
			throw std::runtime_error ( "Failed to read file: " + filename );
			}

		file.close ();

		CE_CORE_DEBUG ( "Successfully read file: {} ({} bytes)", filename, fileSize );
		return buffer;
		}

	CEVulkanBasePipeline::CEVulkanBasePipeline ( CEVulkanContext * context, const PipelineConfig & config )
		: m_Context ( context ), m_Config ( config )
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

   // Правильные пути к скомпилированным шейдерам
#ifdef _DEBUG
		std::string compiledShadersDir = "E:/Projects/EngineChuddo/bin/Debug/Resources/Shaders/Vulkan/";
		std::string vertPath = compiledShadersDir + "triangle.vert.spv";
		std::string fragPath = compiledShadersDir + "triangle.frag.spv";
#else
		std::string compiledShadersDir = "E:/Projects/EngineChuddo/bin/Release/Resources/Shaders/Vulkan/";
		std::string vertPath = compiledShadersDir + "triangle.vert.spv";
		std::string fragPath = compiledShadersDir + "triangle.frag.spv";

#endif // _DEBUG


		CE_CORE_DEBUG ( "Trying vertex shader: {}", vertPath );
		CE_CORE_DEBUG ( "Trying fragment shader: {}", fragPath );

		// Проверяем существование файлов
		std::ifstream vertTest ( vertPath );
		std::ifstream fragTest ( fragPath );

		if (vertTest.is_open ())
			{
			CE_CORE_DEBUG ( "? Vertex shader file exists" );
			vertTest.close ();
			}
		else
			{
			CE_CORE_ERROR ( "? Vertex shader file not found: {}", vertPath );
			return false;
			}

		if (fragTest.is_open ())
			{
			CE_CORE_DEBUG ( "? Fragment shader file exists" );
			fragTest.close ();
			}
		else
			{
			CE_CORE_ERROR ( "? Fragment shader file not found: {}", fragPath );
			return false;
			}

			// Загружаем шейдеры
		if (!SetVertexShader ( vertPath ))
			{
			CE_CORE_ERROR ( "Failed to load vertex shader" );
			return false;
			}

		if (!SetFragmentShader ( fragPath ))
			{
			CE_CORE_ERROR ( "Failed to load fragment shader" );
			return false;
			}

			// Проверяем что шейдеры загружены
		if (m_VertexShaderCode.empty () || m_FragmentShaderCode.empty ())
			{
			CE_CORE_ERROR ( "Shader codes are empty for pipeline '{}'", m_Config.Name );
			return false;
			}

		CE_CORE_DEBUG ( "Shaders loaded successfully - vertex: {} words, fragment: {} words",
						m_VertexShaderCode.size (), m_FragmentShaderCode.size () );

		CE_CORE_DEBUG ( "Shader validation:" );
		CE_CORE_DEBUG ( "  Vertex shader size: {} bytes", m_VertexShaderCode.size () * sizeof ( uint32_t ) );
		CE_CORE_DEBUG ( "  Fragment shader size: {} bytes", m_FragmentShaderCode.size () * sizeof ( uint32_t ) );

		// Проверка SPIR-V magic number
		if (m_VertexShaderCode.size () > 0)
			{
			CE_CORE_DEBUG ( "  Vertex SPIR-V magic: 0x{:08X}", m_VertexShaderCode[ 0 ] );
			}
		if (m_FragmentShaderCode.size () > 0)
			{
			CE_CORE_DEBUG ( "  Fragment SPIR-V magic: 0x{:08X}", m_FragmentShaderCode[ 0 ] );
			}

		   // Создаем descriptor set layout
		if (!CreateDescriptorSetLayout ())
			{
			CE_CORE_ERROR ( "Failed to create descriptor set layout" );
			return false;
			}

			// Создаем pipeline layout  
		if (!CreatePipelineLayout ())
			{
			CE_CORE_ERROR ( "Failed to create pipeline layout" );
			return false;
			}

			// Создаем graphics pipeline
		if (!CreateGraphicsPipeline ( renderPass ))
			{
			CE_CORE_ERROR ( "Failed to create graphics pipeline" );
			return false;
			}

		CE_CORE_DEBUG ( "Base pipeline '{}' initialized successfully", m_Config.Name );
		return true;
		}

	void CEVulkanBasePipeline::Shutdown ()
		{
		auto device = m_Context->GetDevice ();

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

		DestroyShaderModules ();

		CE_CORE_DEBUG ( "Base pipeline '{}' shutdown complete", m_Config.Name );
		}

	void CEVulkanBasePipeline::Bind ( VkCommandBuffer commandBuffer )
		{
		if (GraphicsPipeline != VK_NULL_HANDLE)
			{
			vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline );
			}
		}

	void CEVulkanBasePipeline::UpdateGlobalUniforms ( uint32_t currentImage, const Math::Matrix4 & view, const Math::Matrix4 & projection )
		{
			// Base implementation
		CE_CORE_DEBUG ( "Updating global uniforms for pipeline '{}'", m_Config.Name );
		}

	bool CEVulkanBasePipeline::SetVertexShader ( const std::string & shaderPath )
		{
		m_VertexShaderPath = shaderPath;
		CE_CORE_DEBUG ( "Vertex shader path set to: {}", shaderPath );

		// Сразу загружаем SPIR-V код
		m_VertexShaderCode = LoadShader ( shaderPath );
		bool success = !m_VertexShaderCode.empty ();

		if (success)
			{
			CE_CORE_DEBUG ( "Vertex shader loaded successfully: {} words", m_VertexShaderCode.size () );
			}
		else
			{
			CE_CORE_ERROR ( "Failed to load vertex shader: {}", shaderPath );
			}

		return success;
		}

	bool CEVulkanBasePipeline::SetFragmentShader ( const std::string & shaderPath )
		{
		m_FragmentShaderPath = shaderPath;
		CE_CORE_DEBUG ( "Fragment shader path set to: {}", shaderPath );

		// Сразу загружаем SPIR-V код
		m_FragmentShaderCode = LoadShader ( shaderPath );
		bool success = !m_FragmentShaderCode.empty ();

		if (success)
			{
			CE_CORE_DEBUG ( "Fragment shader loaded successfully: {} words", m_FragmentShaderCode.size () );
			}
		else
			{
			CE_CORE_ERROR ( "Failed to load fragment shader: {}", shaderPath );
			}

		return success;
		}

	bool CEVulkanBasePipeline::CompileShaders ()
		{
		CE_CORE_DEBUG ( "Compiling shaders for pipeline '{}'", m_Config.Name );

		bool success = true;

		// Для vertex shader
		if (!m_VertexShaderPath.empty ())
			{
			if (m_VertexShaderPath.find ( ".spv" ) == std::string::npos)
				{
// GLSL файл - компилируем
				m_VertexShaderCode = CompileAndLoadShader ( m_VertexShaderPath );
				}
			else
				{
						 // SPIR-V файл - просто загружаем
				m_VertexShaderCode = LoadShader ( m_VertexShaderPath );
				}

			if (m_VertexShaderCode.empty ())
				{
				CE_CORE_ERROR ( "Failed to load vertex shader: {}", m_VertexShaderPath );
				success = false;
				}
			}

			// Для fragment shader
		if (!m_FragmentShaderPath.empty ())
			{
			if (m_FragmentShaderPath.find ( ".spv" ) == std::string::npos)
				{
// GLSL файл - компилируем
				m_FragmentShaderCode = CompileAndLoadShader ( m_FragmentShaderPath );
				}
			else
				{
						 // SPIR-V файл - просто загружаем
				m_FragmentShaderCode = LoadShader ( m_FragmentShaderPath );
				}

			if (m_FragmentShaderCode.empty ())
				{
				CE_CORE_ERROR ( "Failed to load fragment shader: {}", m_FragmentShaderPath );
				success = false;
				}
			}

		if (success)
			{
			CE_CORE_DEBUG ( "All shaders compiled successfully for pipeline '{}'", m_Config.Name );
			}
		else
			{
			CE_CORE_ERROR ( "Shader compilation failed for pipeline '{}'", m_Config.Name );
			}

		return success;
		}

	bool CEVulkanBasePipeline::ReloadShaders ()
		{
		CE_CORE_DEBUG ( "ReloadShaders called for pipeline '{}'", m_Config.Name );
		return CompileShaders ();
		}

	std::vector<uint32_t> CEVulkanBasePipeline::CompileAndLoadShader ( const std::string & sourcePath )
		{
			// Для GLSL файлов используем компилятор
		if (sourcePath.find ( ".spv" ) == std::string::npos)
			{
// Это GLSL файл - нужно скомпилировать
			std::string outputPath = sourcePath + ".spv";

			CE::ShaderCompiler compiler;
			auto result = compiler.CompileShader ( sourcePath, outputPath );

			if (result.success)
				{
				CE_CORE_DEBUG ( "Successfully compiled GLSL to SPIR-V: {}", outputPath );
				return LoadShader ( outputPath );
				}
			else
				{
				CE_CORE_ERROR ( "Failed to compile GLSL shader: {}", result.message );
				return std::vector<uint32_t> ();
				}
			}

			// Уже SPIR-V файл - просто загружаем
		return LoadShader ( sourcePath );
		}

	std::vector<uint32_t> CEVulkanBasePipeline::LoadShader ( const std::string & filename )
		{
		CE_CORE_DEBUG ( "Loading shader: {}", filename );

		try
			{
		   // Пытаемся загрузить SPIR-V файл
			auto buffer = ReadFile ( filename );

			// Проверяем что размер корректен для SPIR-V
			if (buffer.size () % sizeof ( uint32_t ) != 0)
				{
				CE_CORE_ERROR ( "Invalid SPIR-V file size: {} bytes (not divisible by 4)", buffer.size () );
				return std::vector<uint32_t> ();
				}

			if (buffer.empty ())
				{
				CE_CORE_ERROR ( "Shader file is empty: {}", filename );
				return std::vector<uint32_t> ();
				}

			std::vector<uint32_t> spirv ( buffer.size () / sizeof ( uint32_t ) );
			memcpy ( spirv.data (), buffer.data (), buffer.size () );

			CE_CORE_DEBUG ( "Loaded SPIR-V shader: {} ({} bytes, {} words)",
							filename, buffer.size (), spirv.size () );

			   // Проверяем SPIR-V magic number
			if (spirv.size () > 0 && spirv[ 0 ] == 0x07230203)
				{
				CE_CORE_DEBUG ( "SPIR-V magic number verified" );
				}
			else
				{
				CE_CORE_ERROR ( "SPIR-V magic number mismatch - expected 0x07230203, got 0x{:08X}",
								spirv.size () > 0 ? spirv[ 0 ] : 0 );
				return std::vector<uint32_t> ();
				}

			return spirv;
			}
			catch (const std::exception & e)
				{
				CE_CORE_ERROR ( "Failed to load shader {}: {}", filename, e.what () );
				return std::vector<uint32_t> ();
				}
		}

	void CEVulkanBasePipeline::DestroyShaderModules ()
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

	bool CEVulkanBasePipeline::CreateDescriptorSetLayout ()
		{
			// Basic descriptor set layout with one uniform buffer
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

		if (vkCreateDescriptorSetLayout ( m_Context->GetDevice (), &layoutInfo, nullptr, &m_DescriptorSetLayout ) != VK_SUCCESS)
			{
			CE_CORE_ERROR ( "Failed to create descriptor set layout for pipeline '{}'", m_Config.Name );
			return false;
			}

		return true;
		}

	bool CEVulkanBasePipeline::CreatePipelineLayout ()
		{
			// Define push constant range for transformation matrices
		VkPushConstantRange pushConstantRange {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof ( MatrixPushConstants ); // 128 bytes for 2 matrices

		VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;  // ADD THIS
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;  // ADD THIS

		if (vkCreatePipelineLayout ( m_Context->GetDevice (), &pipelineLayoutInfo, nullptr, &PipelineLayout ) != VK_SUCCESS)
			{
			CE_CORE_ERROR ( "Failed to create pipeline layout for pipeline '{}'", m_Config.Name );
			return false;
			}

		return true;
		}

	bool CEVulkanBasePipeline::CreateGraphicsPipeline ( VkRenderPass renderPass )
		{
		if (!m_Context)
			{
			CE_CORE_ERROR ( "Context is null in CreateGraphicsPipeline for '{}'", m_Config.Name );
			return false;
			}

		auto device = m_Context->GetDevice ();
		if (device == VK_NULL_HANDLE)
			{
			CE_CORE_ERROR ( "Device is null in CreateGraphicsPipeline for '{}'", m_Config.Name );
			return false;
			}

		if (renderPass == VK_NULL_HANDLE)
			{
			CE_CORE_ERROR ( "RenderPass is null in CreateGraphicsPipeline for '{}'", m_Config.Name );
			return false;
			}

		if (PipelineLayout == VK_NULL_HANDLE)
			{
			CE_CORE_ERROR ( "PipelineLayout is null in CreateGraphicsPipeline for '{}'", m_Config.Name );
			return false;
			}

		CE_CORE_DEBUG ( "Creating graphics pipeline for '{}'", m_Config.Name );

		try
			{
		   // Проверяем что шейдеры не пустые
			if (m_VertexShaderCode.empty ())
				{
				CE_CORE_ERROR ( "Vertex shader is empty for pipeline '{}'", m_Config.Name );
				return false;
				}

			if (m_FragmentShaderCode.empty ())
				{
				CE_CORE_ERROR ( "Fragment shader is empty for pipeline '{}'", m_Config.Name );
				return false;
				}

			CE_CORE_DEBUG ( "Creating shader modules - vertex: {} words, fragment: {} words",
							m_VertexShaderCode.size (), m_FragmentShaderCode.size () );

			   // Создаем шейдерные модули
			VkShaderModule vertexShaderModule = CreateShaderModule ( m_VertexShaderCode );
			VkShaderModule fragmentShaderModule = CreateShaderModule ( m_FragmentShaderCode );

			if (vertexShaderModule == VK_NULL_HANDLE)
				{
				CE_CORE_ERROR ( "Failed to create vertex shader module" );
				return false;
				}

			if (fragmentShaderModule == VK_NULL_HANDLE)
				{
				CE_CORE_ERROR ( "Failed to create fragment shader module" );
				vkDestroyShaderModule ( m_Context->GetDevice (), vertexShaderModule, nullptr );
				return false;
				}

			CE_CORE_DEBUG ( "Shader modules created successfully" );

			// Настраиваем шейдерные стадии
			VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertexShaderModule;
			vertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragmentShaderModule;
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages [] = { vertShaderStageInfo, fragShaderStageInfo };

			// Vertex input - УПРОЩЕННАЯ ВЕРСИЯ для диагностики
			auto bindDescs = CEMeshComponent::Vertex::GetBindingDescription ();
			auto atribDesc = CEMeshComponent::Vertex::GetAttributeDescriptions ();

			//CE_CORE_DEBUG ( "Vertex binding: stride={}, rate={}",
						//	bindDescs.stride, bindDescs.inputRate );
			//CE_CORE_DEBUG ( "Vertex attributes count: {}", atribDesc.size () );

			VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &bindDescs;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( atribDesc.size () );
			vertexInputInfo.pVertexAttributeDescriptions = atribDesc.data ();

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
			colorBlendAttachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlending {};
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
			VkPipelineDynamicStateCreateInfo dynamicState {};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast< uint32_t >( dynamicStates.size () );
			dynamicState.pDynamicStates = dynamicStates.data ();

			// Depth stencil - ОТКЛЮЧЕНО для диагностики
			VkPipelineDepthStencilStateCreateInfo depthStencil {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;

			// Создаем информацию о пайплайне
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
			pipelineInfo.basePipelineIndex = -1;

			CE_CORE_DEBUG ( "Calling vkCreateGraphicsPipelines..." );

			// Создаем пайплайн
			VkResult result = vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GraphicsPipeline );

			// Уничтожаем шейдерные модули
			vkDestroyShaderModule ( device, fragmentShaderModule, nullptr );
			vkDestroyShaderModule ( device, vertexShaderModule, nullptr );

			if (result != VK_SUCCESS)
				{
				CE_CORE_ERROR ( "Failed to create graphics pipeline for '{}': {}",
								m_Config.Name, static_cast< int >( result ) );
				return false;
				}

			CE_CORE_DEBUG ( "Graphics pipeline created successfully for '{}', handle: {}",
							m_Config.Name, ( void * ) GraphicsPipeline );
			return true;
			}
			catch (const std::exception & e)
				{
				CE_CORE_ERROR ( "Exception in CreateGraphicsPipeline for '{}': {}", m_Config.Name, e.what () );
				return false;
				}
		}

	VkShaderModule CEVulkanBasePipeline::CreateShaderModule ( const std::vector<uint32_t> & code )
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
	}