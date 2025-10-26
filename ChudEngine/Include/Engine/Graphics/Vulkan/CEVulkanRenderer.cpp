// Runtime/Renderer/Vulkan/CEVulkanRenderer.cpp
#include "CEVulkanRenderer.hpp"
#include "CEVulkanContext.hpp"
#include "CEVulkanSwapchain.hpp"
#include "CEVulkanSync.hpp"
#include "CEVulkanPipeline.hpp"
#include "CEVulkanCommandBuffer.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Math/Matrix.hpp"
#include "Math/MathUtils.hpp" 
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
    {
    void CEVulkanRenderer::SetCameraParameters ( const Math::Vector3 & position,
                                                 const Math::Vector3 & target,
                                                 float fov )
        {
        float aspectRatio = static_cast< float >( Window->GetWidth () ) / static_cast< float >( Window->GetHeight () );

   // Используйте перспективную проекцию вместо ортографической
        ProjectionMatrix = Math::Matrix4::Perspective (
            Math::ToRadians ( 60.0f ),  // field of view
            aspectRatio,            // aspect ratio  
            0.1f,                   // near plane
            100.0f                  // far plane
        );

        ViewMatrix = Math::Matrix4::LookAt (
            Math::Vector3 ( 0.0f, 0.0f, 1.0f ),
            Math::Vector3 ( 0.0f, 0.0f, 0.0f ),
            Math::Vector3 ( 0.0f, 1.0f, 0.0f )
        );
        }

    void CEVulkanRenderer::ReloadShaders ()
        {
        if (!Initialized || !PipelineManager) return;

        CE_CORE_DEBUG ( "Reloading shaders..." );

        // Ждем завершения операций устройства
        vkDeviceWaitIdle ( Context->GetDevice () );

        // Сохраняем текущие матрицы
        auto viewMatrix = ViewMatrix;
        auto projMatrix = ProjectionMatrix;

        // Перезагружаем все пайплайны через менеджер
        PipelineManager->ReloadAllPipelines ();

        // Обновляем матрицы во всех пайплайнах
        auto staticMeshPipeline = PipelineManager->GetStaticMeshPipeline ();
        if (staticMeshPipeline)
            {
            staticMeshPipeline->SetViewMatrix ( viewMatrix );
            staticMeshPipeline->SetProjectionMatrix ( projMatrix );
            }

        auto skeletalMeshPipeline = PipelineManager->GetSkeletalMeshPipeline ();
        if (skeletalMeshPipeline)
            {
            skeletalMeshPipeline->SetViewMatrix ( viewMatrix );
            skeletalMeshPipeline->SetProjectionMatrix ( projMatrix );
            }

        auto lightPipeline = PipelineManager->GetLightPipeline ();
        if (lightPipeline)
            {
            lightPipeline->SetViewMatrix ( viewMatrix );
            lightPipeline->SetProjectionMatrix ( projMatrix );
            }

        CE_CORE_DEBUG ( "Shaders reloaded successfully" );
        }

    void CEVulkanRenderer::SetCameraViewMatrix ( const Math::Matrix4 & viewMatrix )
        {
        ViewMatrix = viewMatrix;
        }

    void CEVulkanRenderer::SetCameraProjectionMatrix ( const Math::Matrix4 & projectionMatrix )
        {
        ProjectionMatrix = projectionMatrix;
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
        if (Initialized)
            {
            CE_CORE_WARN ( "VulkanRenderer already initialized" );
            return false;
            }

        Window = window;
        CE_CORE_DEBUG ( "Initializing Vulkan renderer" );

        try
            {
                // Initialize context
            Context = std::make_unique<CEVulkanContext> ();
            if (!Context->Initialize ( window ))
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan context" );
                return false;
                }

                // Initialize swapchain
            Swapchain = std::make_unique<CEVulkanSwapchain> ( Context.get () );
            if (!Swapchain->Initialize ( window ))
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan swapchain" );
                return false;
                }

                // Initialize sync objects
            SyncManager = std::make_unique<CEVulkanSync> ( Context.get () );
            if (!SyncManager->Initialize ())
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan sync objects" );
                return false;
                }

            PipelineManager = std::make_unique<CEVulkanPipelineManager> ( Context.get () );
            CE_CORE_DEBUG ( "Creating pipeline manager..." );
            if (!PipelineManager->Initialize ( Swapchain->GetRenderPass () ))
                {
                CE_CORE_ERROR ( "Failed to initialize pipeline manager" );
                return false;
                }
            CE_CORE_DEBUG ( "Pipeline manager initialized" );

                // Initialize command buffer
            CommandBuffer = std::make_unique<CEVulkanCommandBuffer> ( Context.get () );
            if (!CommandBuffer->Initialize ())
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan command buffer" );
                return false;
                }

                // Устанавливаем начальные матрицы камеры
            SetCameraParameters (
               Math::Vector3 ( .0f, 0.0f, 2.0f ), // position
               Math::Vector3 ( 0.0f, 0.0f, 0.0f ), // target
                60.0f // fov
            );

            Initialized = true;
            CE_CORE_DEBUG ( "Vulkan renderer initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan renderer: {}", e.what () );
                return false;
                }
        }

    void CEVulkanRenderer::Shutdown ()
        {
        if (!Initialized) return;

        CE_CORE_DEBUG ( "Shutting down Vulkan renderer" );

        auto device = Context ? Context->GetDevice () : VK_NULL_HANDLE;
        if (device != VK_NULL_HANDLE)
            {
            vkDeviceWaitIdle ( device );
            }

            // Cleanup in reverse order of initialization
        if (CommandBuffer)
            {
            CommandBuffer->Shutdown ();
            CommandBuffer.reset ();
            }

        if (PipelineManager)
            {
            PipelineManager->Shutdown ();
            PipelineManager.reset ();
            }

        if (SyncManager)
            {
            SyncManager->Shutdown ();
            SyncManager.reset ();
            }

        if (Swapchain)
            {
            Swapchain->Shutdown ();
            Swapchain.reset ();
            }

        if (Context)
            {
            Context->Shutdown ();
            Context.reset ();
            }

        Initialized = false;
        CE_CORE_DEBUG ( "Vulkan renderer shutdown complete" );
        }

    void CEVulkanRenderer::RenderFrame ()
        {
        if (!Initialized)
            {
            CE_CORE_WARN ( "Renderer not initialized" );
            return;
            }

         // Диагностика матриц (можно убрать после отладки)
        static int frameCount = 0;
        if (frameCount % 60 == 0) // Каждые 60 кадров
            {
            DebugPrintMatrices ();
            }
        frameCount++;

        try
            {
                // 1. Ждем завершения предыдущего кадра
            if (!SyncManager->WaitForFrameFence ())
                {
                CE_CORE_ERROR ( "Failed to wait for frame fence" );
                return;
                }

                // 2. Сбрасываем fence для текущего кадра
            if (!SyncManager->ResetFrameFence ())
                {
                CE_CORE_ERROR ( "Failed to reset frame fence" );
                return;
                }

                // 3. Получаем следующий image из swapchain
            VkResult acquireResult = Swapchain->AcquireNextImage ( SyncManager->GetImageAvailableSemaphore () );

            if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
                {
                CE_CORE_DEBUG ( "Swapchain out of date, recreating..." );
                OnWindowResized ();
                return;
                }
            else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
                {
                CE_CORE_ERROR ( "Failed to acquire swapchain image: {}", static_cast< int >( acquireResult ) );
                return;
                }

                // 4. Сбрасываем command buffer
            CommandBuffer->Reset ();

            if (!CommandBuffer->IsReadyForRecording ())
                {
                CE_CORE_ERROR ( "Command buffer is not ready for recording!" );
                return;
                }

                // 5. Записываем command buffer
            CommandBuffer->BeginRecording ();
            RecordCommandBuffer ( CommandBuffer->GetCommandBuffer (), Swapchain->GetCurrentImageIndex () );
            CommandBuffer->EndRecording ();

            // 6. Отправляем command buffer
            VkResult submitResult = Swapchain->SubmitCommandBuffer (
                CommandBuffer->GetCommandBuffer (),
                SyncManager->GetImageAvailableSemaphore (),
                SyncManager->GetRenderFinishedSemaphore (),
                SyncManager->GetInFlightFence ()
            );

            if (submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR)
                {
                CE_CORE_DEBUG ( "Swapchain needs recreation after submit" );
                OnWindowResized ();
                }
            else if (submitResult != VK_SUCCESS)
                {
                CE_CORE_ERROR ( "Failed to submit command buffer: {}", static_cast< int >( submitResult ) );
                }

                // 7. Переходим к следующему кадру
            SyncManager->AdvanceFrame ();
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Error during frame rendering: {}", e.what () );
                }
        }

    void CEVulkanRenderer::OnWindowResized ()
        {
        if (!Initialized) return;

        CE_CORE_DEBUG ( "Recreating swapchain due to window resize" );

        // Ждем завершения операций устройства
        vkDeviceWaitIdle ( Context->GetDevice () );

        // Получаем новый размер окна
        int width, height;
        glfwGetFramebufferSize ( Window->GetNativeWindow (), &width, &height );

        while (width == 0 || height == 0)
            {
            glfwGetFramebufferSize ( Window->GetNativeWindow (), &width, &height );
            glfwWaitEvents ();

            // Если окно закрыто, выходим
            if (glfwWindowShouldClose ( Window->GetNativeWindow () ))
                return;
            }

            // Уничтожаем старый swapchain
        Swapchain->Shutdown ();

        // Пересоздаем
        if (!Swapchain->Initialize ( Window ))
            {
            CE_CORE_ERROR ( "Failed to recreate swapchain!" );
            return;
            }

            // Пересоздаем все пайплайны с новым render pass
        PipelineManager->Shutdown ();
        if (!PipelineManager->Initialize ( Swapchain->GetRenderPass () ))
            {
            CE_CORE_ERROR ( "Failed to recreate pipelines!" );
            return;
            }

        CE_CORE_DEBUG ( "Swapchain and pipelines recreated successfully" );
        }

    void CEVulkanRenderer::RenderWorldMeshes ( VkCommandBuffer commandBuffer )
        {
        if (m_WorldRenderer)
            {
            m_WorldRenderer->Render ( commandBuffer );
            }
        else
            {
            CE_DEBUG ( "No world renderer available, using fallback triangle" );
           // RenderFallbackTriangle ( commandBuffer );
            }
        }

    void CEVulkanRenderer::RenderWorld ( CEWorld * world )
        {
        if (!world || !Initialized) return;

        // Сохраняем ссылку на мир
        m_CurrentWorld = world;

        // Создаем world renderer если нужно
        if (!m_WorldRenderer)
            {
            m_WorldRenderer = std::make_unique<CEWorldRenderer> ( this );
            }
        m_WorldRenderer->SetWorld ( world );

        // Основной рендеринг кадра
        RenderFrame ();

        // Отладочная информация
        auto actors = world->GetActors ();
        int meshCount = 0;

        for (auto * actor : actors)
            {
            if (actor)
                {
                auto meshComps = actor->GetComponentsOfType<CEMeshComponent> ();
                meshCount += static_cast< int >( meshComps.size () );  // Явное приведение типа
                }
            }

        CE_DEBUG ( "World has {} actors with {} mesh components", actors.size (), meshCount );
        }

    void CEVulkanRenderer::RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex )
        {
            // Проверяем критически важные указатели
        if (!Swapchain || !PipelineManager || !SyncManager)
            {
            CE_CORE_ERROR ( "Critical components not initialized in RecordCommandBuffer" );
            return;
            }

        VkRenderPass renderPass = Swapchain->GetRenderPass ();
        if (renderPass == VK_NULL_HANDLE)
            {
            CE_CORE_ERROR ( "Render pass is null in RecordCommandBuffer" );
            return;
            }

            // Begin render pass
        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;

        // Получаем framebuffers по ссылке, а не копированием - ИСПРАВЛЕНО
        const auto & framebuffers = Swapchain->GetFramebuffers (); // Добавлен const&
        if (imageIndex >= framebuffers.Size ())
            {
            CE_CORE_ERROR ( "Invalid image index: {} (max: {})", imageIndex, framebuffers.Size () );
            return;
            }
        renderPassInfo.framebuffer = framebuffers[ imageIndex ];

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = Swapchain->GetExtent ();

        VkClearValue clearValues[ 2 ];
        clearValues[ 0 ].color = { {.20f, .20f, .2f, 1.0f} };
        clearValues[ 1 ].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        // Set dynamic viewport and scissor
        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast< float >( Swapchain->GetExtent ().width );
        viewport.height = static_cast< float >( Swapchain->GetExtent ().height );
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 5.0f;
        vkCmdSetViewport ( commandBuffer, 0, 1, &viewport );

        VkRect2D scissor {};
        scissor.offset = { 0, 0 };
        scissor.extent = Swapchain->GetExtent ();
        vkCmdSetScissor ( commandBuffer, 0, 1, &scissor );

        

        

        // Рендерим мир с мешами вместо fallback треугольника
        RenderWorldMeshes ( commandBuffer );
        // End render pass
        vkCmdEndRenderPass ( commandBuffer );
        }

    void CEVulkanRenderer::RenderFallbackTriangle ( VkCommandBuffer commandBuffer )
        {
        if (!PipelineManager)
            {
            CE_CORE_ERROR ( "PipelineManager is null in RenderFallbackTriangle" );
            return;
            }

        auto staticMeshPipeline = PipelineManager->GetStaticMeshPipeline ();
        if (!staticMeshPipeline)
            {
            CE_CORE_ERROR ( "StaticMeshPipeline is null in RenderFallbackTriangle" );
            return;
            }

        if (staticMeshPipeline->GetPipeline () == VK_NULL_HANDLE)
            {
            CE_CORE_ERROR ( "StaticMeshPipeline VkPipeline is null in RenderFallbackTriangle" );
            return;
            }

        CE_CORE_DEBUG ( "Binding static mesh pipeline for fallback triangle" );
        CE_CORE_DEBUG ( "StaticMeshPipeline valid: {}",
                        ( staticMeshPipeline->GetPipeline () != VK_NULL_HANDLE ) );

           // Добавьте проверку pipeline layout
        if (staticMeshPipeline->GetLayout () == VK_NULL_HANDLE)
            {
            CE_CORE_ERROR ( "Pipeline layout is null!" );
            return;
            }

        staticMeshPipeline->Bind ( commandBuffer );
        vkCmdDraw ( commandBuffer, 3, 1, 0, 0 );
        }
    }