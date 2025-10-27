// Runtime/Renderer/Vulkan/CEVulkanRenderer.cpp
#include "CEVulkanRenderer.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Core/Application/CEApplication.hpp"
#include "Core/CEObject/CEWorld.hpp"
#include "Math/MathUtils.hpp"
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
    {
    void CEVulkanRenderer::SetCameraParameters ( const Math::Vector3 & position,
                                                 const Math::Vector3 & target,
                                                 float fov )
        {
        m_CameraPosition = position;
        m_CameraTarget = target;
        m_CameraFOV = fov;

        if (m_Window)
            {
            float aspectRatio = static_cast< float >( m_Window->GetWidth () ) / static_cast< float >( m_Window->GetHeight () );

            // Используем перспективную проекцию
            m_ProjectionMatrix = Math::Matrix4::Perspective (
                Math::ToRadians ( fov ),
                aspectRatio,
                0.1f,
                100.0f
            );

            m_ViewMatrix = Math::Matrix4::LookAt (
                position,
                target,
                Math::Vector3 ( 0.0f, 1.0f, 0.0f )
            );
            }
        }

    void CEVulkanRenderer::SetCameraViewMatrix ( const Math::Matrix4 & viewMatrix )
        {
        m_ViewMatrix = viewMatrix;
        }

    void CEVulkanRenderer::SetCameraProjectionMatrix ( const Math::Matrix4 & projectionMatrix )
        {
        m_ProjectionMatrix = projectionMatrix;
        }

    void CEVulkanRenderer::RenderWorld ( CEWorld * world )
        {
        if (!world || !m_Initialized) return;

        // Для совместимости - просто рендерим кадр
        RenderFrame ();

        // Логируем информацию о мире
        auto actors = world->GetActors ();
        int meshCount = 0;

        for (auto * actor : actors)
            {
            if (actor)
                {
                    // Подсчитываем меш компоненты
                    // auto meshComps = actor->GetComponentsOfType<CEMeshComponent>();
                    // meshCount += static_cast<int>(meshComps.size());
                meshCount++; // временная заглушка
                }
            }

        CE_DEBUG ( "World has {} actors with {} mesh components", actors.size (), meshCount );
        }

    CEVulkanRenderer::CEVulkanRenderer ()
        {
        CE_CORE_DEBUG ( "Vulkan renderer created" );
        }

    CEVulkanRenderer::~CEVulkanRenderer ()
        {
        Shutdown ();
        }

    bool CEVulkanRenderer::Initialize ( CEWindow * window )
        {
        if (m_Initialized)
            {
            CE_CORE_WARN ( "VulkanRenderer already initialized" );
            return false;
            }

        m_Window = window;
        CE_CORE_DEBUG ( "Initializing Vulkan renderer" );

        try
            {
                // 1. Initialize context
            m_Context = std::make_unique<CEVulkanContext> ();
            if (!m_Context->Initialize ( window ))
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan context" );
                return false;
                }

                // 2. Create resource managers
            m_ShaderManager = std::make_unique<CEVulkanShaderManager> ( m_Context.get () );
            m_ResourceManager = std::make_unique<CEVulkanResourceManager> ( m_Context.get () );

            // 3. Create swapchain
            m_Swapchain = std::make_unique<CEVulkanSwapchain> ();
            if (!m_Swapchain->Initialize ( m_Context.get (), window ))
                {
                CE_CORE_ERROR ( "Failed to initialize swapchain" );
                return false;
                }

                // 4. Create pipeline manager
            m_PipelineManager = std::make_unique<CEVulkanPipelineManager> ( m_Context.get (), m_ShaderManager.get () );
            if (!m_PipelineManager->Initialize ( m_Swapchain->GetRenderPass () ))
                {
                CE_CORE_ERROR ( "Failed to initialize pipeline manager" );
                return false;
                }

                // 5. Create sync objects
            m_SyncManager = std::make_unique<CEVulkanSync> ();
            if (!m_SyncManager->Initialize ( m_Context.get (), m_Swapchain->GetMaxFramesInFlight () ))
                {
                CE_CORE_ERROR ( "Failed to initialize sync objects" );
                return false;
                }

                // 6. Create command buffers
            m_CommandBuffer = std::make_unique<CEVulkanCommandBuffer> ();
            if (!m_CommandBuffer->Initialize ( m_Context.get (), m_Swapchain->GetMaxFramesInFlight () ))
                {
                CE_CORE_ERROR ( "Failed to initialize command buffers" );
                return false;
                }

                // 7. Set default camera
            SetCameraParameters (
                Math::Vector3 ( 0.0f, 0.0f, 2.0f ),
                Math::Vector3 ( 0.0f, 0.0f, 0.0f ),
                60.0f
            );

            m_Initialized = true;
            CE_CORE_INFO ( "Vulkan renderer initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Exception during Vulkan renderer initialization: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanRenderer::Shutdown ()
        {
        if (!m_Initialized) return;

        CE_CORE_DEBUG ( "Shutting down Vulkan renderer" );

        // Wait for device idle
        if (m_Context)
            {
            vkDeviceWaitIdle ( m_Context->GetDevice () );
            }

            // Cleanup in reverse order
        m_CommandBuffer.reset ();
        m_SyncManager.reset ();
        m_PipelineManager.reset ();
        m_Swapchain.reset ();
        m_ResourceManager.reset ();
        m_ShaderManager.reset ();
        m_Context.reset ();

        m_Initialized = false;
        CE_CORE_DEBUG ( "Vulkan renderer shutdown complete" );
        }

    void CEVulkanRenderer::RenderFrame ()
        {
        if (!m_Initialized) return;

        try
            {
                // 1. Acquire next image
            uint32_t imageIndex;
            VkResult result = m_SyncManager->AcquireNextImage ( m_Swapchain.get (), &imageIndex );

            if (result == VK_ERROR_OUT_OF_DATE_KHR)
                {
                RecreateSwapchain ();
                return;
                }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                {
                CE_CORE_ERROR ( "Failed to acquire swapchain image: {}", static_cast< int >( result ) );
                return;
                }

                // 2. Reset and begin command buffer
            m_CommandBuffer->ResetCurrent ();
            m_CommandBuffer->BeginRecording ();

            // 3. Record commands
            RecordCommandBuffer ( m_CommandBuffer->GetCurrent (), imageIndex );
            m_CommandBuffer->EndRecording ();

            // 4. Submit frame
            if (m_SyncManager->SubmitFrame ( m_CommandBuffer.get (), m_Swapchain.get (), imageIndex ))
                {
                    // 5. Present
                result = m_Swapchain->Present ( imageIndex, m_SyncManager->GetCurrentRenderFinishedSemaphore () );

                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
                    {
                    RecreateSwapchain ();
                    }
                else if (result != VK_SUCCESS)
                    {
                    CE_CORE_ERROR ( "Failed to present swapchain image: {}", static_cast< int >( result ) );
                    }
                }

                // 6. Advance to next frame
            m_SyncManager->AdvanceFrame ();
            m_CommandBuffer->AdvanceFrame ();
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Exception during frame rendering: {}", e.what () );
                }
        }

    void CEVulkanRenderer::OnWindowResized ()
        {
        if (m_Initialized)
            {
            CE_CORE_DEBUG ( "Window resized, recreating swapchain" );
            RecreateSwapchain ();
            }
        }

    void CEVulkanRenderer::ReloadShaders ()
        {
        if (m_ShaderManager && m_PipelineManager)
            {
            CE_CORE_DEBUG ( "Reloading all shaders..." );
            m_ShaderManager->ReloadAllShaders ();
            m_PipelineManager->ReloadAllPipelines ();
            CE_CORE_DEBUG ( "Shaders reloaded successfully" );
            }
        }

    bool CEVulkanRenderer::IsMatrixInitialized ( const Math::Matrix4 & matrix )
        {
         // Проверяем что матрица не нулевая (простая проверка)
        for (int i = 0; i < 4; ++i)
            {
            for (int j = 0; j < 4; ++j)
                {
                if (matrix.At ( i, j ) != 0.0f)
                    return true;
                }
            }
        return false;
        }

    void CEVulkanRenderer::RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex )
        {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;

        if (vkBeginCommandBuffer ( commandBuffer, &beginInfo ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to begin recording command buffer" );
            }

            // Begin render pass
        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_Swapchain->GetRenderPass ();
        renderPassInfo.framebuffer = m_Swapchain->GetFramebuffer ( imageIndex );
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_Swapchain->GetExtent ();

        std::array<VkClearValue, 2> clearValues {};
        clearValues[ 0 ].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[ 1 ].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size () );
        renderPassInfo.pClearValues = clearValues.data ();

        vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        // Set dynamic viewport and scissor
        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast< float >( m_Swapchain->GetExtent ().width );
        viewport.height = static_cast< float >( m_Swapchain->GetExtent ().height );
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport ( commandBuffer, 0, 1, &viewport );

        VkRect2D scissor {};
        scissor.offset = { 0, 0 };
        scissor.extent = m_Swapchain->GetExtent ();
        vkCmdSetScissor ( commandBuffer, 0, 1, &scissor );

        // Render fallback triangle
        RenderFallbackTriangle ( commandBuffer );

        vkCmdEndRenderPass ( commandBuffer );

        if (vkEndCommandBuffer ( commandBuffer ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to record command buffer" );
            }
        }

    void CEVulkanRenderer::RenderFallbackTriangle ( VkCommandBuffer commandBuffer )
        {
        auto pipeline = m_PipelineManager->GetDefaultPipeline ();
        if (!pipeline)
            {
            CE_CORE_WARN ( "No default pipeline available for fallback rendering" );
            return;
            }

        pipeline->Bind ( commandBuffer );
        vkCmdDraw ( commandBuffer, 3, 1, 0, 0 );
        }

    void CEVulkanRenderer::RecreateSwapchain ()
        {
        vkDeviceWaitIdle ( m_Context->GetDevice () );

        // Store old swapchain
        auto oldSwapchain = std::move ( m_Swapchain );

        // Create new swapchain
        m_Swapchain = std::make_unique<CEVulkanSwapchain> ();
        if (!m_Swapchain->Initialize ( m_Context.get (), m_Window, oldSwapchain.get () ))
            {
            CE_CORE_ERROR ( "Failed to recreate swapchain" );
            return;
            }

            // Recreate pipelines if render pass changed
        if (m_PipelineManager && oldSwapchain)
            {
            if (m_Swapchain->GetRenderPass () != oldSwapchain->GetRenderPass ())
                {
                m_PipelineManager->RecreatePipelines ( m_Swapchain->GetRenderPass () );
                }
            }

        CE_CORE_DEBUG ( "Swapchain recreated successfully" );
        }
    }