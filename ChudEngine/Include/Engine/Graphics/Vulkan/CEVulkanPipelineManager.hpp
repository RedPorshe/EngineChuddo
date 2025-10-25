// Runtime/Renderer/Vulkan/CEVulkanPipelineManager.hpp
#pragma once
#include <unordered_map>
#include <memory>
#include "CEVulkanBasePipeline.hpp"
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
        Count
        };

    class CEVulkanPipelineManager
        {
        public:
            CEVulkanPipelineManager ( CEVulkanContext * context );
            ~CEVulkanPipelineManager ();

            bool Initialize ( VkRenderPass mainRenderPass );
            void Shutdown ();

            CEVulkanBasePipeline * GetPipeline ( PipelineType type );
            bool ReloadPipeline ( PipelineType type );
            void ReloadAllPipelines ();

          

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

        private:
            bool CreatePipeline ( PipelineType type, VkRenderPass renderPass );
            bool CreateMinimalPipeline ( PipelineType type, VkRenderPass renderPass );
            CEVulkanContext * m_Context;
            std::unordered_map<PipelineType, std::unique_ptr<CEVulkanBasePipeline>> m_Pipelines;
            VkRenderPass m_MainRenderPass = VK_NULL_HANDLE;
            VkRenderPass m_UIRenderPass = VK_NULL_HANDLE;
        };
    }