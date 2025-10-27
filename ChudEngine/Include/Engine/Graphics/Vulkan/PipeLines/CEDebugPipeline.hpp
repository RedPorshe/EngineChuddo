// Graphics/Vulkan/Pipelines/CEDebugPipeline.hpp
#pragma once
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CEDebugPipeline : public CEVulkanBasePipeline
        {
        public:
            CEDebugPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CEDebugPipeline () override = default;

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;
        };
    }