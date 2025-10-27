// Graphics/Vulkan/Managers/CEVulkanPipelineManager.hpp
#pragma once
#include <unordered_map>
#include <memory>
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"

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
        Debug,
        Shadow,
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

            CEVulkanBasePipeline * GetPipeline ( PipelineType type );
            CEVulkanBasePipeline * GetDefaultPipeline () { return GetPipeline ( PipelineType::StaticMesh ); }

            template<typename T>
            T * GetPipelineAs ( PipelineType type ) {
                return static_cast< T * >( GetPipeline ( type ) );
                }

            bool ReloadPipeline ( PipelineType type );
            void ReloadAllPipelines ();

        private:
            bool CreatePipeline ( PipelineType type, VkRenderPass renderPass );

            CEVulkanContext * m_Context = nullptr;
            CEVulkanShaderManager * m_ShaderManager = nullptr;
            std::unordered_map<PipelineType, std::unique_ptr<CEVulkanBasePipeline>> m_Pipelines;
            VkRenderPass m_MainRenderPass = VK_NULL_HANDLE;
        };
    }