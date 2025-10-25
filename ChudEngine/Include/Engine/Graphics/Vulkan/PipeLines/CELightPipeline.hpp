// Runtime/Renderer/Vulkan/Pipelines/CELightPipeline.hpp
#pragma once
#include "../CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CELightPipeline : public CEVulkanBasePipeline
        {
        public:
            CELightPipeline ( CEVulkanContext * context );

            bool Initialize ( VkRenderPass renderPass ) override;

            // Light specific methods
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

            std::vector<std::unique_ptr<CEVulkanBuffer>> m_LightUniformBuffers;
            std::vector<void *> m_LightUniformBuffersMapped;
        };
    }