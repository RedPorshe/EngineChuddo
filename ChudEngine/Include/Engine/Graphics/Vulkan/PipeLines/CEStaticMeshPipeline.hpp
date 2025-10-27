// Graphics/Vulkan/Pipelines/CEStaticMeshPipeline.hpp
#pragma once
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CEStaticMeshPipeline : public CEVulkanBasePipeline
        {
        public:
            CEStaticMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CEVulkanBasePipeline () override = default;

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;
        };
    }