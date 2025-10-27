// Runtime/Renderer/Vulkan/CEVulkanPipelineManager.hpp
#pragma once
#include <unordered_map>
#include <memory>
#include "CEVulkanBasePipeline.hpp"
#include "CEVulkanShaderManager.hpp"
#include "Pipelines/CEStaticMeshPipeline.hpp"
#include "Pipelines/CESkeletalMeshPipeline.hpp"
#include "Pipelines/CELightPipeline.hpp"
#include "Pipelines/CEPostProcessPipeline.hpp"

namespace CE
    {
    enum class PipelineType
        {
        StaticMesh,
        SkeletalMesh,
        Light,
        PostProcess,
        Skybox,
        UI,
        Default,
        Count
        };

    class CEVulkanPipelineManager
        {
        public:
            CEVulkanPipelineManager ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CEVulkanPipelineManager ();

            bool Initialize ( VkRenderPass mainRenderPass );
            void Shutdown ();
            void RecreatePipelines ( VkRenderPass newRenderPass );

            // Pipeline management
            CEVulkanBasePipeline * GetPipeline ( PipelineType type );
            CEVulkanBasePipeline * GetDefaultPipeline () { return GetPipeline ( PipelineType::Default ); }

            // Convenience accessors
            CEStaticMeshPipeline * GetStaticMeshPipeline () {
                return static_cast< CEStaticMeshPipeline * >( GetPipeline ( PipelineType::StaticMesh ) );
                }

            CESkeletalMeshPipeline * GetSkeletalMeshPipeline () {
                return static_cast< CESkeletalMeshPipeline * >( GetPipeline ( PipelineType::SkeletalMesh ) );
                }

            CELightPipeline * GetLightPipeline () {
                return static_cast< CELightPipeline * >( GetPipeline ( PipelineType::Light ) );
                }

            CEPostProcessPipeline * GetPostProcessPipeline () {
                return static_cast< CEPostProcessPipeline * >( GetPipeline ( PipelineType::PostProcess ) );
                }

            bool ReloadPipeline ( PipelineType type );
            void ReloadAllPipelines ();

        private:
            bool CreatePipeline ( PipelineType type, VkRenderPass renderPass );
            bool CreateDefaultPipeline ( VkRenderPass renderPass );

            CEVulkanContext * m_Context = nullptr;
            CEVulkanShaderManager * m_ShaderManager = nullptr;
            std::unordered_map<PipelineType, std::unique_ptr<CEVulkanBasePipeline>> m_Pipelines;
            VkRenderPass m_MainRenderPass = VK_NULL_HANDLE;
        };
    }