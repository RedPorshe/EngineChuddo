#include "Graphics/Vulkan/Managers/CEVulkanPipelineManager.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Graphics/Vulkan/Pipelines/CEStaticMeshPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CESkeletalMeshPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CELightPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CEPostProcessPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CEDebugPipeline.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanPipelineManager::CEVulkanPipelineManager ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : m_Context ( context )
        , m_ShaderManager ( shaderManager )
        , m_MainRenderPass ( VK_NULL_HANDLE )
        {
        }

    CEVulkanPipelineManager::~CEVulkanPipelineManager ()
        {
        Shutdown ();
        }

    bool CEVulkanPipelineManager::Initialize ( VkRenderPass mainRenderPass )
        {
        if (m_MainRenderPass != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Pipeline manager already initialized" );
            return true;
            }

        m_MainRenderPass = mainRenderPass;

        try
            {
           // Create all pipeline types
            for (int i = 0; i < static_cast< int > ( PipelineType::Count ); i++)
                {
                PipelineType type = static_cast< PipelineType > ( i );
                if (!CreatePipeline ( type, m_MainRenderPass ))
                    {
                    CE_CORE_WARN ( "Failed to create pipeline type: {}", static_cast< int > ( type ) );
                    }
                }

            CE_CORE_INFO ( "Pipeline manager initialized successfully with {} pipelines", m_Pipelines.size () );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize pipeline manager: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanPipelineManager::Shutdown ()
        {
        m_Pipelines.clear ();
        m_MainRenderPass = VK_NULL_HANDLE;
        CE_CORE_DEBUG ( "Pipeline manager shut down" );
        }

    void CEVulkanPipelineManager::RecreatePipelines ( VkRenderPass newRenderPass )
        {
        m_MainRenderPass = newRenderPass;

        CE_CORE_INFO ( "Recreating all pipelines..." );

        for (auto & [type, pipeline] : m_Pipelines)
            {
            if (pipeline)
                {
                pipeline->Shutdown ();
                if (!pipeline->Initialize ( m_MainRenderPass ))
                    {
                    CE_CORE_ERROR ( "Failed to recreate pipeline type: {}", static_cast< int >( type ) );
                    }
                }
            }
        }

    CEVulkanBasePipeline * CEVulkanPipelineManager::GetPipeline ( PipelineType type )
        {
        auto it = m_Pipelines.find ( type );
        if (it != m_Pipelines.end () && it->second)
            {
            return it->second.get ();
            }

        CE_CORE_WARN ( "Pipeline not found for type: {}", static_cast< int >( type ) );
        return nullptr;
        }

    bool CEVulkanPipelineManager::ReloadPipeline ( PipelineType type )
        {
        auto it = m_Pipelines.find ( type );
        if (it != m_Pipelines.end () && it->second)
            {
            CE_CORE_INFO ( "Reloading pipeline type: {}", static_cast< int >( type ) );
            return it->second->ReloadShaders ();
            }

        CE_CORE_WARN ( "Cannot reload pipeline - not found: {}", static_cast< int >( type ) );
        return false;
        }

    void CEVulkanPipelineManager::ReloadAllPipelines ()
        {
        CE_CORE_INFO ( "Reloading all pipelines..." );

        for (auto & [type, pipeline] : m_Pipelines)
            {
            if (pipeline)
                {
                if (!pipeline->ReloadShaders ())
                    {
                    CE_CORE_ERROR ( "Failed to reload pipeline type: {}", static_cast< int >( type ) );
                    }
                }
            }
        }

    bool CEVulkanPipelineManager::CreatePipeline ( PipelineType type, VkRenderPass renderPass )
        {
        if (m_Pipelines.find ( type ) != m_Pipelines.end ())
            {
            CE_CORE_WARN ( "Pipeline already exists for type: {}", static_cast< int >( type ) );
            return true;
            }

        std::unique_ptr<CEVulkanBasePipeline> pipeline;

        try
            {
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

                    case PipelineType::Debug:
                        pipeline = std::make_unique<CEDebugPipeline> ( m_Context, m_ShaderManager );
                        break;

                    case PipelineType::Skybox:
                    case PipelineType::UI:
                        // TODO: Implement these pipeline types
                        CE_CORE_WARN ( "Pipeline type not implemented: {}", static_cast< int >( type ) );
                        return false;

                    default:
                        CE_CORE_ERROR ( "Unknown pipeline type: {}", static_cast< int >( type ) );
                        return false;
                }

            if (pipeline && pipeline->Initialize ( renderPass ))
                {
                m_Pipelines[ type ] = std::move ( pipeline );
                CE_CORE_DEBUG ( "Created pipeline type: {}", static_cast< int >( type ) );
                return true;
                }
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to create pipeline type {}: {}", static_cast< int >( type ), e.what () );
                }

            return false;
        }
    }