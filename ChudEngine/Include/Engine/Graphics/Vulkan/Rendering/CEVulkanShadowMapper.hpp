// Graphics/Vulkan/Rendering/CEVulkanShadowMapper.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanPipelineManager;
    class CEVulkanTextureManager;
    class CEVulkanBuffer;

    class CEVulkanShadowMapper
        {
        public:
            CEVulkanShadowMapper ();
            ~CEVulkanShadowMapper ();

            bool Initialize ( CEVulkanContext * context,
                              CEVulkanPipelineManager * pipelineManager,
                              CEVulkanTextureManager * textureManager );
            void Shutdown ();

            void SetLightDirection ( const Math::Vector3 & direction );
            void SetShadowMapResolution ( uint32_t resolution );

            void RenderShadowMap ( VkCommandBuffer commandBuffer,
                                   const std::vector<std::shared_ptr<class CEVulkanMesh>> & meshes );

            VkImageView GetShadowMapView () const { return m_ShadowMapView; }
            const Math::Matrix4 & GetLightSpaceMatrix () const { return m_LightSpaceMatrix; }

        private:
            bool CreateShadowMap ();
            bool CreatePipeline ();
            void UpdateLightSpaceMatrix ();

            CEVulkanContext * m_Context = nullptr;
            CEVulkanPipelineManager * m_PipelineManager = nullptr;
            CEVulkanTextureManager * m_TextureManager = nullptr;

            VkImage m_ShadowMap = VK_NULL_HANDLE;
            VkDeviceMemory m_ShadowMapMemory = VK_NULL_HANDLE;
            VkImageView m_ShadowMapView = VK_NULL_HANDLE;
            VkFramebuffer m_ShadowMapFramebuffer = VK_NULL_HANDLE;
            VkRenderPass m_ShadowRenderPass = VK_NULL_HANDLE;
            VkPipeline m_ShadowPipeline = VK_NULL_HANDLE;
            VkPipelineLayout m_ShadowPipelineLayout = VK_NULL_HANDLE;

            Math::Vector3 m_LightDirection = { 0.0f, -1.0f, 0.0f };
            Math::Matrix4 m_LightSpaceMatrix;
            uint32_t m_ShadowMapResolution = 2048;

            bool m_Initialized = false;
        };
    }