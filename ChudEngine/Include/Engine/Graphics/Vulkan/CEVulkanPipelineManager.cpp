// Runtime/Renderer/Vulkan/CEVulkanPipelineManager.cpp
#include "CEVulkanPipelineManager.hpp"
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
    {
    CEVulkanPipelineManager::CEVulkanPipelineManager ( CEVulkanContext * context )
        : m_Context ( context )
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

        CE_CORE_DEBUG ( "Initializing pipeline manager with {} pipeline types",
                        static_cast< int >( PipelineType::Count ) );

          // Временно создаем только StaticMesh пайплайн с минимальной конфигурацией
        if (!CreateMinimalPipeline ( PipelineType::StaticMesh, m_MainRenderPass ))
            {
            CE_CORE_ERROR ( "Failed to create minimal StaticMesh pipeline" );
            return false;
            }

        CE_CORE_DEBUG ( "Pipeline manager initialized successfully with {} pipelines",
                        m_Pipelines.size () );
        return true;
        }

    bool CEVulkanPipelineManager::CreateMinimalPipeline ( PipelineType type, VkRenderPass renderPass )
        {
        CE_CORE_DEBUG ( "Creating minimal pipeline type: {}", static_cast< int >( type ) );

        // Создаем минимальную конфигурацию
        PipelineConfig config;
        config.Name = "StaticMesh";
        config.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.PolygonMode = VK_POLYGON_MODE_FILL;
        config.CullMode = VK_CULL_MODE_BACK_BIT;
        config.FrontFace = VK_FRONT_FACE_CLOCKWISE;
        config.DepthTest = VK_TRUE;  // Временно отключаем
        config.DepthWrite = VK_TRUE; // Временно отключаем
        config.BlendEnable = VK_TRUE;

        std::unique_ptr<CEVulkanBasePipeline> pipeline = std::make_unique<CEVulkanBasePipeline> ( m_Context, config );

        if (pipeline->Initialize ( renderPass ))
            {
            m_Pipelines[ type ] = std::move ( pipeline );
            CE_CORE_DEBUG ( "Minimal pipeline type {} created successfully: {}",
                            static_cast< int >( type ), m_Pipelines[ type ]->GetName () );
            return true;
            }
        else
            {
            CE_CORE_ERROR ( "Failed to initialize minimal pipeline type {}", static_cast< int >( type ) );
            return false;
            }
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

    CEVulkanBasePipeline * CEVulkanPipelineManager::GetPipeline ( PipelineType type )
        {
        auto it = m_Pipelines.find ( type );
        if (it != m_Pipelines.end ())
            {
            return it->second.get ();
            }

        CE_CORE_WARN ( "Pipeline type {} not found", static_cast< int >( type ) );
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

        // Сохраняем текущий render pass
        VkRenderPass renderPass = pipeline->GetLayout () ? m_MainRenderPass : VK_NULL_HANDLE;

        // Пересоздаем пайплайн
        pipeline->Shutdown ();
        if (!pipeline->Initialize ( renderPass ))
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
                    pipeline = std::make_unique<CEStaticMeshPipeline> ( m_Context );
                    break;

                case PipelineType::SkeletalMesh:
                    pipeline = std::make_unique<CESkeletalMeshPipeline> ( m_Context );
                    break;

                case PipelineType::Light:
                    pipeline = std::make_unique<CELightPipeline> ( m_Context );
                    break;

                case PipelineType::PostProcess:
                    pipeline = std::make_unique<CEPostProcessPipeline> ( m_Context );
                    break;

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
    }