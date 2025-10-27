#pragma once
#include <unordered_map>
#include <memory>
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"

#include "Graphics/Vulkan/PipeLines/CEStaticMeshPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CESkeletalMeshPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CELightPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CEPostProcessPipeline.hpp"

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