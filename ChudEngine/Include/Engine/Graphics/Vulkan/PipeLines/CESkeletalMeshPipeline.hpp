// Graphics/Vulkan/Pipelines/CESkeletalMeshPipeline.hpp
#pragma once
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"
#include <vector>

namespace CE
    {
    class CESkeletalMeshPipeline : public CEVulkanBasePipeline
        {
        public:
            CESkeletalMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CESkeletalMeshPipeline () override = default;

            void UpdateBoneMatrices ( uint32_t currentImage, const std::vector<Math::Matrix4> & boneMatrices );

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;

            struct SkeletalUniforms
                {
                alignas( 16 ) Math::Matrix4 bones[ 100 ];
                };

            std::vector<void *> m_BoneUniformBuffersMapped;
        };
    }