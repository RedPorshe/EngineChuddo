// Runtime/Renderer/Vulkan/CEVulkanRenderer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "../CERenderer.hpp"
#include "Graphics/CEWorldRenderer.hpp"
#include "Core/Application/CEApplication.hpp"
#include "Core/CEObject/CEWorld.hpp"
#include "CEVulkanPipelineManager.hpp"
#include <memory>

namespace CE
    {
    class CEWindow;
    class CEVulkanContext;
    class CEVulkanSwapchain;
    class CEVulkanSync;
    class CEVulkanCommandBuffer;
    class CEApplication;

    class CEVulkanRenderer : public CERenderer
        {
        public:
            CEVulkanRenderer ();
            ~CEVulkanRenderer () override;

            void SetCameraParameters ( const CE::Math::Vector3 & position,
                                       const CE::Math::Vector3 & target,
                                       float fov );

            void SetCameraViewMatrix ( const CE::Math::Matrix4 & viewMatrix );
            void SetCameraProjectionMatrix ( const CE::Math::Matrix4 & projectionMatrix );

            const Math::Matrix4 & GetViewMatrix () const { return ViewMatrix; }
            const Math::Matrix4 & GetProjectionMatrix () const { return ProjectionMatrix; }
            const Math::Vector3 & GetCameraPosition () const { return CameraPosition; }

            CEVulkanPipelineManager * GetPipelineManager () { return PipelineManager.get (); }
            void SetCurrentApplication ( CEApplication * app ) { CurrentApplication = app; }

            void ReloadShaders ();
            bool Initialize ( CEWindow * window ) override;
            void Shutdown () override;
            void RenderFrame () override;
            void OnWindowResized () override;

            CEVulkanContext * GetContext () const { return Context.get (); }
            void RenderWorld ( CEWorld * world );

        private:
            void RenderFallbackTriangle ( VkCommandBuffer commandBuffer );
            void RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex );

            void RenderWorldMeshes ( VkCommandBuffer commandBuffer );

            CEWindow * Window = nullptr;
            CEApplication * CurrentApplication = nullptr;
            std::unique_ptr<CEVulkanContext> Context;
            std::unique_ptr<CEVulkanSwapchain> Swapchain;
            std::unique_ptr<CEVulkanSync> SyncManager;
            std::unique_ptr<CEVulkanCommandBuffer> CommandBuffer;
            std::unique_ptr<CEVulkanPipelineManager> PipelineManager;
            std::unique_ptr<CEWorldRenderer> m_WorldRenderer;


            bool Initialized = false;

            // Параметры камеры
            Math::Vector3 CameraPosition;
            Math::Vector3 CameraTarget;
            float CameraFOV;
            Math::Matrix4 ViewMatrix;
            Math::Matrix4 ProjectionMatrix;

            CEWorld* m_CurrentWorld;
        };
    }