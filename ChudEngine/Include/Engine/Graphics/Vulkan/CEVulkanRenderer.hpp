// Runtime/Renderer/Vulkan/CEVulkanRenderer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "../CERenderer.hpp"
#include "Core/Application/CEApplication.hpp"
#include "Core/CEObject/CEWorld.hpp"
#include <memory>

namespace CE
    {
    class CEWindow;
    class CEVulkanContext;
    class CEVulkanSwapchain;
    class CEVulkanSync;
    class CEVulkanPipeline;
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

            void SetCurrentApplication ( CEApplication * app ) { CurrentApplication = app; }
            void ReloadShaders ();
            bool Initialize ( CEWindow * window ) override;
            void Shutdown () override;
            void RenderFrame () override;
            void OnWindowResized () override;
            CEVulkanContext * GetContext () const { return Context.get (); }
            void RenderWorld ( CEWorld * world );
        private:
            void RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex );
            void RenderActor ( CEActor * actor, VkCommandBuffer commandBuffer );
            CEWindow * Window = nullptr;
            CEApplication * CurrentApplication = nullptr;
            std::unique_ptr<CEVulkanContext> Context;
            std::unique_ptr<CEVulkanSwapchain> Swapchain;
            std::unique_ptr<CEVulkanSync> SyncManager;
            std::unique_ptr<CEVulkanPipeline> Pipeline;
            std::unique_ptr<CEVulkanCommandBuffer> CommandBuffer;
            bool Initialized = false;

             // Параметры камеры
            Math::Vector3 CameraPosition;
            Math::Vector3 CameraTarget;
            float CameraFOV;
            Math::Matrix4 ViewMatrix;
            Math::Matrix4 ProjectionMatrix;
        };
    }