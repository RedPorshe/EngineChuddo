// Runtime/Renderer/Vulkan/Pipelines/CESkeletalMeshPipeline.hpp
#pragma once
#include "../CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CESkeletalMeshPipeline : public CEVulkanBasePipeline
        {
        public:
            CESkeletalMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CESkeletalMeshPipeline () = default;
           

            // Skeletal animation specific methods
            void UpdateBoneMatrices ( uint32_t currentImage, const std::vector<Math::Matrix4> & boneMatrices );

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;

            struct SkeletalUniforms
                {
                alignas( 16 ) Math::Matrix4 bones[ 100 ]; // Max bones
                };

         //   std::vector<std::unique_ptr<CEVulkanBuffer>> m_BoneUniformBuffers;
            std::vector<void *> m_BoneUniformBuffersMapped;
        };
    }