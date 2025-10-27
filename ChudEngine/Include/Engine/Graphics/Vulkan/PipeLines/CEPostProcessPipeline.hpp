#pragma once
#include "Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp"

namespace CE
    {
    class CEPostProcessPipeline : public CEVulkanBasePipeline
        {
        public:
            CEPostProcessPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CEPostProcessPipeline () = default;

          

            // Post-processing specific methods
            void SetInputTexture ( VkImageView textureView, VkSampler sampler );
            void UpdatePostProcessParams ( uint32_t currentImage, float time, const Math::Vector2 & screenSize );

        private:
            bool CreateDescriptorSetLayout () override;
            bool CreateGraphicsPipeline ( VkRenderPass renderPass ) override;

            VkDescriptorPool m_TextureDescriptorPool = VK_NULL_HANDLE;
            VkDescriptorSet m_TextureDescriptorSet = VK_NULL_HANDLE;

            struct PostProcessUniforms
                {
                alignas( 4 ) float time;
                alignas( 8 ) Math::Vector2 screenSize;
                };
        };
    }