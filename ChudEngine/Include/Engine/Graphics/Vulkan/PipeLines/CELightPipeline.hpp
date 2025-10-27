#pragma once
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CELightPipeline : public CEVulkanBasePipeline
        {
        public:
            CELightPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CELightPipeline () override = default;

            void UpdateLightData ( uint32_t currentImage, const Math::Vector3 & position,
                                   const Math::Vector3 & color, float intensity );

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;

            struct LightUniforms
                {
                alignas( 16 ) Math::Vector3 position;
                alignas( 16 ) Math::Vector3 color;
                alignas( 4 ) float intensity;
                };

            std::vector<void *> m_LightUniformBuffersMapped;
        };
    }