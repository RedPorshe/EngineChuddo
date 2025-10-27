// Runtime/Renderer/Vulkan/CEVulkanRenderer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "../CERenderer.hpp"
#include "CEVulkanContext.hpp"
#include "CEVulkanSwapchain.hpp"
#include "CEVulkanSync.hpp"
#include "CEVulkanCommandBuffer.hpp"
#include "CEVulkanPipelineManager.hpp"
#include "CEVulkanShaderManager.hpp"
#include "CEVulkanResourceManager.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include <memory>
#include <array>

namespace CE
    {
    class CEWindow;
    class CEApplication;
    class CEWorld;

    class CEVulkanRenderer : public CERenderer
        {
        public:
            CEVulkanRenderer ();
            ~CEVulkanRenderer () override;

            // Основной интерфейс CERenderer
            bool Initialize ( CEWindow * window ) override;
            void Shutdown () override;
            void RenderFrame () override;
            void OnWindowResized () override;

            // Camera methods
            void SetCameraParameters ( const Math::Vector3 & position,
                                       const Math::Vector3 & target,
                                       float fov );
            void SetCameraViewMatrix ( const Math::Matrix4 & viewMatrix );
            void SetCameraProjectionMatrix ( const Math::Matrix4 & projectionMatrix );
            void RenderWorld ( CEWorld * world );

            // Application management
            void SetCurrentApplication ( CEApplication * app ) { m_CurrentApplication = app; }

            // Shader management
            void ReloadShaders ();

            // Getters
            CEVulkanContext * GetContext () const { return m_Context.get (); }
            CEVulkanPipelineManager * GetPipelineManager () { return m_PipelineManager.get (); }
            CEVulkanShaderManager * GetShaderManager () { return m_ShaderManager.get (); }
            CEVulkanResourceManager * GetResourceManager () { return m_ResourceManager.get (); }

            const Math::Matrix4 & GetViewMatrix () const { return m_ViewMatrix; }
            const Math::Matrix4 & GetProjectionMatrix () const { return m_ProjectionMatrix; }
            const Math::Vector3 & GetCameraPosition () const { return m_CameraPosition; }

            bool IsMatrixInitialized ( const Math::Matrix4 & matrix );

            void DebugPrintMatrices () const
                {
                CE_CORE_DEBUG ( "=== MATRIX DEBUG ===" );
                CE_CORE_DEBUG ( "View Matrix initialized: {}", IsMatrixInitialized ( m_ViewMatrix ) );
                CE_CORE_DEBUG ( "Projection Matrix initialized: {}", IsMatrixInitialized ( m_ProjectionMatrix ) );
                }

        private:
            // Основные компоненты
            CEWindow * m_Window = nullptr;
            CEApplication * m_CurrentApplication = nullptr;
            std::unique_ptr<CEVulkanContext> m_Context;
            std::unique_ptr<CEVulkanSwapchain> m_Swapchain;
            std::unique_ptr<CEVulkanSync> m_SyncManager;
            std::unique_ptr<CEVulkanCommandBuffer> m_CommandBuffer;

            // Менеджеры ресурсов
            std::unique_ptr<CEVulkanShaderManager> m_ShaderManager;
            std::unique_ptr<CEVulkanResourceManager> m_ResourceManager;
            std::unique_ptr<CEVulkanPipelineManager> m_PipelineManager;

            // Состояние
            bool m_Initialized = false;

            // Camera state
            Math::Vector3 m_CameraPosition;
            Math::Vector3 m_CameraTarget;
            float m_CameraFOV = 60.0f;
            Math::Matrix4 m_ViewMatrix;
            Math::Matrix4 m_ProjectionMatrix;

            // Вспомогательные методы
            void RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex );
            void RenderFallbackTriangle ( VkCommandBuffer commandBuffer );
            void RecreateSwapchain ();
        };
    }