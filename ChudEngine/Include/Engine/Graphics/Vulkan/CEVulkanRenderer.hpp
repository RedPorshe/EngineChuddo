// Graphics/Vulkan/CEVulkanRenderer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include "Graphics/CERenderer.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

namespace CE
    {
    class CEWindow;
    class CEApplication;
    class CEWorld;

    class CEVulkanContext;
    class CEVulkanSwapchain;
    class CEVulkanSync;
    class CEVulkanCommandBuffer;
    class CEVulkanPipelineManager;
    class CEVulkanShaderManager;
    class CEVulkanResourceManager;
    class CEVulkanTextureManager;
    class CEVulkanCamera;
    class CEVulkanSceneRenderer;
    class CEVulkanDebugRenderer;
    class CEVulkanStats;

    class CEVulkanRenderer : public CERenderer
        {
        public:
            CEVulkanRenderer ();
            ~CEVulkanRenderer () override;

            bool Initialize ( CEWindow * window ) override;
            void Shutdown () override;
            void RenderFrame () override;
            void OnWindowResized () override;

            void SetCurrentApplication ( CEApplication * app ) { m_CurrentApplication = app; }
            void SetWorld ( CEWorld * world );
            void ReloadShaders ();

            CEVulkanContext * GetContext () const { return m_Context.get (); }
            CEVulkanPipelineManager * GetPipelineManager () { return m_PipelineManager.get (); }
            CEVulkanShaderManager * GetShaderManager () { return m_ShaderManager.get (); }
            CEVulkanResourceManager * GetResourceManager () { return m_ResourceManager.get (); }
            CEVulkanTextureManager * GetTextureManager () { return m_TextureManager.get (); }
            CEVulkanCamera * GetCamera () { return m_Camera.get (); }
            CEVulkanSceneRenderer * GetSceneRenderer () { return m_SceneRenderer.get (); }

            const Math::Matrix4 & GetViewMatrix () const;
            const Math::Matrix4 & GetProjectionMatrix () const;
            const Math::Vector3 & GetCameraPosition () const;

        private:
            void RecreateSwapchain ();
            void RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex );
            void RenderFallbackTriangle ( VkCommandBuffer commandBuffer );

            CEWindow * m_Window = nullptr;
            CEApplication * m_CurrentApplication = nullptr;

            std::unique_ptr<CEVulkanContext> m_Context;
            std::unique_ptr<CEVulkanSwapchain> m_Swapchain;
            std::unique_ptr<CEVulkanSync> m_SyncManager;
            std::unique_ptr<CEVulkanCommandBuffer> m_CommandBuffer;

            std::unique_ptr<CEVulkanShaderManager> m_ShaderManager;
            std::unique_ptr<CEVulkanResourceManager> m_ResourceManager;
            std::unique_ptr<CEVulkanTextureManager> m_TextureManager;
            std::unique_ptr<CEVulkanPipelineManager> m_PipelineManager;

            std::unique_ptr<CEVulkanCamera> m_Camera;
            std::unique_ptr<CEVulkanSceneRenderer> m_SceneRenderer;
            std::unique_ptr<CEVulkanDebugRenderer> m_DebugRenderer;
            std::unique_ptr<CEVulkanStats> m_Stats;

            bool m_Initialized = false;
        };
    }