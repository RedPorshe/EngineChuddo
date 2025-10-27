// Graphics/Vulkan/Rendering/CEVulkanPostProcessor.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanPipelineManager;
    class CEVulkanTextureManager;

    struct PostProcessEffect
        {
        std::string name;
        bool enabled = true;
        std::shared_ptr<class CEVulkanMaterial> material;
        };

    class CEVulkanPostProcessor
        {
        public:
            CEVulkanPostProcessor ();
            ~CEVulkanPostProcessor ();

            bool Initialize ( CEVulkanContext * context,
                              CEVulkanPipelineManager * pipelineManager,
                              CEVulkanTextureManager * textureManager );
            void Shutdown ();

            void AddEffect ( const PostProcessEffect & effect );
            void RemoveEffect ( const std::string & name );
            void SetEffectEnabled ( const std::string & name, bool enabled );

            void ApplyEffects ( VkCommandBuffer commandBuffer, VkImageView inputImage );
            VkImageView GetFinalImage () const;

            void Resize ( uint32_t width, uint32_t height );

        private:
            bool CreateRenderTargets ();
            bool CreateFullscreenQuad ();

            CEVulkanContext * m_Context = nullptr;
            CEVulkanPipelineManager * m_PipelineManager = nullptr;
            CEVulkanTextureManager * m_TextureManager = nullptr;

            std::vector<PostProcessEffect> m_Effects;
            std::vector<std::shared_ptr<class CEVulkanTexture>> m_RenderTargets;
            std::unique_ptr<class CEVulkanBuffer> m_FullscreenQuadBuffer;

            uint32_t m_Width = 0;
            uint32_t m_Height = 0;
            bool m_Initialized = false;
        };
    }