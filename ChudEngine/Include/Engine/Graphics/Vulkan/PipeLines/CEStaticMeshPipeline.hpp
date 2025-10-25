// Runtime/Renderer/Vulkan/Pipelines/CEStaticMeshPipeline.hpp
#pragma once
#include "../CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CEStaticMeshPipeline : public CEVulkanBasePipeline
        {
        public:
            CEStaticMeshPipeline ( CEVulkanContext * context );

            bool Initialize ( VkRenderPass renderPass ) override;

            // Static mesh specific methods
            void UpdatePerObjectUniforms ( uint32_t currentImage, uint32_t objectIndex,
                                           const Math::Matrix4 & modelMatrix, const Math::Vector4 & color );
            void BindMeshData ( VkCommandBuffer commandBuffer, VkBuffer vertexBuffer,
                                VkBuffer indexBuffer, uint32_t indexCount );

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreatePipelineLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;

            struct PerObjectUniforms
                {
                alignas( 16 ) Math::Matrix4 model;
                alignas( 16 ) Math::Vector4 color;
                };

            std::vector<std::unique_ptr<CEVulkanBuffer>> m_PerObjectUniformBuffers;
            std::vector<void *> m_PerObjectUniformBuffersMapped;
            VkDeviceSize m_DynamicAlignment = 0;
            static const uint32_t MAX_OBJECTS = 1000;
        };
    }