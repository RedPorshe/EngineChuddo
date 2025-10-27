// Runtime/Renderer/Vulkan/CEVulkanPipelineManager.cpp
#include "CEVulkanPipelineManager.hpp"
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
    {
    CEVulkanPipelineManager::CEVulkanPipelineManager ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : m_Context ( context )
        , m_ShaderManager ( shaderManager )
        {
        CE_CORE_DEBUG ( "Vulkan pipeline manager created" );
        }

    CEVulkanPipelineManager::~CEVulkanPipelineManager ()
        {
        Shutdown ();
        }

    bool CEVulkanPipelineManager::Initialize ( VkRenderPass mainRenderPass )
        {
        m_MainRenderPass = mainRenderPass;

        CE_CORE_DEBUG ( "Initializing pipeline manager" );

        // Create default pipeline first
        if (!CreateDefaultPipeline ( mainRenderPass ))
            {
            CE_CORE_ERROR ( "Failed to create default pipeline" );
            return false;
            }

        CE_CORE_DEBUG ( "Pipeline manager initialized successfully with {} pipelines", m_Pipelines.size () );
        return true;
        }

    void CEVulkanPipelineManager::Shutdown ()
        {
        CE_CORE_DEBUG ( "Shutting down pipeline manager" );

        for (auto & [type, pipeline] : m_Pipelines)
            {
            if (pipeline)
                {
                pipeline->Shutdown ();
                }
            }
        m_Pipelines.clear ();

        CE_CORE_DEBUG ( "Pipeline manager shutdown complete" );
        }

    void CEVulkanPipelineManager::RecreatePipelines ( VkRenderPass newRenderPass )
        {
        CE_CORE_DEBUG ( "Recreating all pipelines for new render pass" );

        m_MainRenderPass = newRenderPass;

        for (auto & [type, pipeline] : m_Pipelines)
            {
            if (pipeline)
                {
                pipeline->Shutdown ();
                pipeline->Initialize ( newRenderPass );
                }
            }

        CE_CORE_DEBUG ( "All pipelines recreated" );
        }

    CEVulkanBasePipeline * CEVulkanPipelineManager::GetPipeline ( PipelineType type )
        {
        auto it = m_Pipelines.find ( type );
        if (it != m_Pipelines.end ())
            {
            return it->second.get ();
            }

            // Try to create the pipeline if it doesn't exist
        if (CreatePipeline ( type, m_MainRenderPass ))
            {
            return m_Pipelines[ type ].get ();
            }

        CE_CORE_WARN ( "Pipeline type {} not found and could not be created", static_cast< int >( type ) );
        return nullptr;
        }

    bool CEVulkanPipelineManager::ReloadPipeline ( PipelineType type )
        {
        auto pipeline = GetPipeline ( type );
        if (!pipeline)
            {
            CE_CORE_ERROR ( "Cannot reload pipeline type {} - not found", static_cast< int >( type ) );
            return false;
            }

        CE_CORE_DEBUG ( "Reloading pipeline: {}", pipeline->GetName () );

        pipeline->Shutdown ();
        if (!pipeline->Initialize ( m_MainRenderPass ))
            {
            CE_CORE_ERROR ( "Failed to reload pipeline type {}", static_cast< int >( type ) );
            return false;
            }

        CE_CORE_DEBUG ( "Pipeline reloaded successfully: {}", pipeline->GetName () );
        return true;
        }

    void CEVulkanPipelineManager::ReloadAllPipelines ()
        {
        CE_CORE_DEBUG ( "Reloading all pipelines" );

        for (auto & [type, pipeline] : m_Pipelines)
            {
            if (pipeline)
                {
                CE_CORE_DEBUG ( "Reloading pipeline type: {}", static_cast< int >( type ) );
                if (!ReloadPipeline ( type ))
                    {
                    CE_CORE_ERROR ( "Failed to reload pipeline type {}", static_cast< int >( type ) );
                    }
                }
            }

        CE_CORE_DEBUG ( "All pipelines reloaded" );
        }

    bool CEVulkanPipelineManager::CreatePipeline ( PipelineType type, VkRenderPass renderPass )
        {
        CE_CORE_DEBUG ( "Creating pipeline type: {}", static_cast< int >( type ) );

        std::unique_ptr<CEVulkanBasePipeline> pipeline;

        switch (type)
            {
                case PipelineType::StaticMesh:
                    pipeline = std::make_unique<CEStaticMeshPipeline> ( m_Context, m_ShaderManager );
                    break;

                case PipelineType::SkeletalMesh:
                    pipeline = std::make_unique<CESkeletalMeshPipeline> ( m_Context, m_ShaderManager );
                    break;

                case PipelineType::Light:
                    pipeline = std::make_unique<CELightPipeline> ( m_Context, m_ShaderManager );
                    break;

                case PipelineType::PostProcess:
                    pipeline = std::make_unique<CEPostProcessPipeline> ( m_Context, m_ShaderManager );
                    break;

                case PipelineType::Default:
                    return CreateDefaultPipeline ( renderPass );

                case PipelineType::Skybox:
                    // TODO: Implement SkyboxPipeline
                    CE_CORE_WARN ( "Skybox pipeline not implemented yet" );
                    return true;

                case PipelineType::UI:
                    // TODO: Implement UIPipeline  
                    CE_CORE_WARN ( "UI pipeline not implemented yet" );
                    return true;

                default:
                    CE_CORE_ERROR ( "Unknown pipeline type: {}", static_cast< int >( type ) );
                    return false;
            }

        if (pipeline)
            {
            if (pipeline->Initialize ( renderPass ))
                {
                m_Pipelines[ type ] = std::move ( pipeline );
                CE_CORE_DEBUG ( "Pipeline type {} created successfully: {}",
                                static_cast< int >( type ), m_Pipelines[ type ]->GetName () );
                return true;
                }
            else
                {
                CE_CORE_ERROR ( "Failed to initialize pipeline type {}", static_cast< int >( type ) );
                return false;
                }
            }

        return false;
        }

    bool CEVulkanPipelineManager::CreateDefaultPipeline ( VkRenderPass renderPass )
        {
        CE_CORE_DEBUG ( "Creating default pipeline" );

        PipelineConfig config;
        config.Name = "DefaultPipeline";
        config.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.PolygonMode = VK_POLYGON_MODE_FILL;
        config.CullMode = VK_CULL_MODE_BACK_BIT;
        config.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        config.DepthTest = true;
        config.DepthWrite = true;
        config.BlendEnable = false;

        auto pipeline = std::make_unique<CEVulkanBasePipeline> ( m_Context, m_ShaderManager, config );

        // Set default shader paths
        std::string vertPath = "Resources/Shaders/Vulkan/triangle.vert";
        std::string fragPath = "Resources/Shaders/Vulkan/triangle.frag";

        if (!pipeline->SetVertexShader ( vertPath ) || !pipeline->SetFragmentShader ( fragPath ))
            {
            CE_CORE_ERROR ( "Failed to set default shaders for pipeline" );
            return false;
            }

        if (pipeline->Initialize ( renderPass ))
            {
            m_Pipelines[ PipelineType::Default ] = std::move ( pipeline );
            CE_CORE_DEBUG ( "Default pipeline created successfully" );
            return true;
            }

        CE_CORE_ERROR ( "Failed to initialize default pipeline" );
        return false;
        }
    }