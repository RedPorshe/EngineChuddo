#include "Graphics/Vulkan/CEVulkanRenderer.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Core/CEVulkanSwapchain.hpp"
#include "Graphics/Vulkan/Core/CEVulkanSync.hpp"
#include "Graphics/Vulkan/Core/CEVulkanCommandBuffer.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanPipelineManager.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanResourceManager.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanTextureManager.hpp"
#include "Graphics/Vulkan/Scene/CEVulkanCamera.hpp"
#include "Graphics/Vulkan/Scene/CEVulkanSceneRenderer.hpp"
#include "Graphics/Vulkan/Debug/CEVulkanDebugRenderer.hpp"
#include "Graphics/Vulkan/Debug/CEVulkanStats.hpp"
#include "Graphics/Vulkan/Rendering/CEVulkanRenderPassManager.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Core/CEObject/CEWorld.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanRenderer::CEVulkanRenderer ()
        : m_Window ( nullptr )
        , m_CurrentApplication ( nullptr )
        , m_Initialized ( false )
        {
        }

    CEVulkanRenderer::~CEVulkanRenderer ()
        {
        Shutdown ();
        }

    bool CEVulkanRenderer::Initialize ( CEWindow * window )
        {
        if (m_Initialized)
            {
            CE_CORE_WARN ( "Renderer already initialized" );
            return true;
            }

        m_Window = window;

        try
            {
           // Initialize core Vulkan components
            m_Context = std::make_unique<CEVulkanContext> ();
            if (!m_Context->Initialize ( window ))
                {
                throw std::runtime_error ( "Failed to initialize Vulkan context" );
                }

                // Initialize managers
            m_ShaderManager = std::make_unique<CEVulkanShaderManager> ( m_Context.get () );
            m_ResourceManager = std::make_unique<CEVulkanResourceManager> ( m_Context.get () );
            m_TextureManager = std::make_unique<CEVulkanTextureManager> ( m_Context.get () );

            // Initialize render pass manager first (needed for swapchain)
            auto renderPassManager = std::make_unique<CEVulkanRenderPassManager> ( m_Context.get () );
            if (!renderPassManager->Initialize ())
                {
                throw std::runtime_error ( "Failed to initialize render pass manager" );
                }

                // Initialize swapchain with main render pass
            m_Swapchain = std::make_unique<CEVulkanSwapchain> ();
            if (!m_Swapchain->Initialize ( m_Context.get (), window ))
                {
                throw std::runtime_error ( "Failed to initialize swapchain" );
                }

                // Set the render pass for swapchain
            m_Swapchain->SetRenderPass ( renderPassManager->GetMainRenderPass () );

            // Initialize pipeline manager with main render pass
            m_PipelineManager = std::make_unique<CEVulkanPipelineManager> ( m_Context.get (), m_ShaderManager.get () );
            if (!m_PipelineManager->Initialize ( renderPassManager->GetMainRenderPass () ))
                {
                throw std::runtime_error ( "Failed to initialize pipeline manager" );
                }

                // Initialize sync and command buffers
            m_SyncManager = std::make_unique<CEVulkanSync> ();
            if (!m_SyncManager->Initialize ( m_Context.get (), m_Swapchain->GetMaxFramesInFlight () ))
                {
                throw std::runtime_error ( "Failed to initialize sync manager" );
                }

            m_CommandBuffer = std::make_unique<CEVulkanCommandBuffer> ();
            if (!m_CommandBuffer->Initialize ( m_Context.get (), m_Swapchain->GetMaxFramesInFlight () ))
                {
                throw std::runtime_error ( "Failed to initialize command buffer" );
                }

                // Initialize camera
            m_Camera = std::make_unique<CEVulkanCamera> ();

            // Initialize scene renderer
            m_SceneRenderer = std::make_unique<CEVulkanSceneRenderer> ( this );
            if (!m_SceneRenderer->Initialize ())
                {
                throw std::runtime_error ( "Failed to initialize scene renderer" );
                }

                // Initialize debug renderer
            m_DebugRenderer = std::make_unique<CEVulkanDebugRenderer> ();
            if (!m_DebugRenderer->Initialize ( m_Context.get (), m_PipelineManager.get (), m_ResourceManager.get () ))
                {
                CE_CORE_WARN ( "Failed to initialize debug renderer, continuing without debug features" );
                }

                // Initialize stats
            m_Stats = std::make_unique<CEVulkanStats> ( m_Context.get () );
            if (!m_Stats->Initialize ())
                {
                CE_CORE_WARN ( "Failed to initialize stats, continuing without statistics" );
                }

            m_Initialized = true;
            CE_CORE_INFO ( "Vulkan renderer initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan renderer: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanRenderer::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            vkDeviceWaitIdle ( m_Context->GetDevice ()->GetDevice () );
            }

        m_Stats.reset ();
        m_DebugRenderer.reset ();
        m_SceneRenderer.reset ();
        m_Camera.reset ();
        m_CommandBuffer.reset ();
        m_SyncManager.reset ();
        m_PipelineManager.reset ();
        m_Swapchain.reset ();
        m_TextureManager.reset ();
        m_ResourceManager.reset ();
        m_ShaderManager.reset ();
        m_Context.reset ();

        m_Initialized = false;
        CE_CORE_INFO ( "Vulkan renderer shut down" );
        }

    void CEVulkanRenderer::RenderFrame ()
        {
        if (!m_Initialized)
            {
            CE_CORE_ERROR ( "Renderer not initialized" );
            return;
            }

        m_Stats->BeginFrame ();

        try
            {
            uint32_t imageIndex = 0;
            VkResult acquireResult = m_SyncManager->AcquireNextImage ( m_Swapchain.get (), &imageIndex );

            if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
                {
                RecreateSwapchain ();
                return;
                }
            else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
                {
                throw std::runtime_error ( "Failed to acquire swap chain image" );
                }

                // Record command buffer
            m_CommandBuffer->ResetCurrent ();
            m_CommandBuffer->BeginRecording ();
            RecordCommandBuffer ( m_CommandBuffer->GetCurrent (), imageIndex );
            m_CommandBuffer->EndRecording ();

            // Submit frame
            if (!m_SyncManager->SubmitFrame ( m_CommandBuffer->GetCurrent (), m_Swapchain.get (), imageIndex ))
                {
                throw std::runtime_error ( "Failed to submit frame" );
                }

            m_SyncManager->AdvanceFrame ();
            m_CommandBuffer->AdvanceFrame ();

            m_Stats->EndFrame ();
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Error during frame rendering: {}", e.what () );
                }
        }

    void CEVulkanRenderer::OnWindowResized ()
        {
        if (m_Initialized)
            {
            CE_CORE_INFO ( "Window resized, recreating swapchain..." );
            RecreateSwapchain ();
            }
        }

    void CEVulkanRenderer::SetWorld ( CEWorld * world )
        {
        if (m_SceneRenderer)
            {
            m_SceneRenderer->SetWorld ( world );
            }
        }

    void CEVulkanRenderer::ReloadShaders ()
        {
        if (m_ShaderManager)
            {
            m_ShaderManager->ReloadAllShaders ();
            }
        if (m_PipelineManager)
            {
            m_PipelineManager->ReloadAllPipelines ();
            }
        CE_CORE_INFO ( "Shaders reloaded" );
        }

    const Math::Matrix4 & CEVulkanRenderer::GetViewMatrix () const
        {
        static Math::Matrix4 identity = Math::Matrix4::Identity ();
        return m_Camera ? m_Camera->GetViewMatrix () : identity;
        }

    const Math::Matrix4 & CEVulkanRenderer::GetProjectionMatrix () const
        {
        static Math::Matrix4 identity = Math::Matrix4::Identity ();
        return m_Camera ? m_Camera->GetProjectionMatrix () : identity;
        }

    const Math::Vector3 & CEVulkanRenderer::GetCameraPosition () const
        {
        static Math::Vector3 zero = Math::Vector3 ( 0.0f );
        return m_Camera ? m_Camera->GetPosition () : zero;
        }

    void CEVulkanRenderer::RecreateSwapchain ()
        {
        if (!m_Initialized) return;

        // Wait for device to be idle before recreating swapchain
        vkDeviceWaitIdle ( m_Context->GetDevice ()->GetDevice () );

        // Store old swapchain for clean recreation
        auto oldSwapchain = std::move ( m_Swapchain );

        // Create new swapchain
        m_Swapchain = std::make_unique<CEVulkanSwapchain> ();
        if (!m_Swapchain->Initialize ( m_Context.get (), m_Window, oldSwapchain.get () ))
            {
            CE_CORE_ERROR ( "Failed to recreate swapchain" );
            return;
            }

            // Recreate pipelines with new swapchain render pass
        if (m_PipelineManager)
            {
            m_PipelineManager->RecreatePipelines ( m_Swapchain->GetRenderPass () );
            }

        CE_CORE_INFO ( "Swapchain recreated successfully" );
        }

    void CEVulkanRenderer::RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex )
        {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer ( commandBuffer, &beginInfo );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to begin recording command buffer" );
            }

            // Begin render pass
        std::array<VkClearValue, 2> clearValues = {};
        clearValues[ 0 ].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[ 1 ].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_Swapchain->GetRenderPass ();
        renderPassInfo.framebuffer = m_Swapchain->GetFramebuffer ( imageIndex );
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_Swapchain->GetExtent ();
        renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size () );
        renderPassInfo.pClearValues = clearValues.data ();

        vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        // Set viewport and scissor
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast< float >( m_Swapchain->GetExtent ().width );
        viewport.height = static_cast< float >( m_Swapchain->GetExtent ().height );
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport ( commandBuffer, 0, 1, &viewport );

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = m_Swapchain->GetExtent ();
        vkCmdSetScissor ( commandBuffer, 0, 1, &scissor );

        // Render scene
        if (m_SceneRenderer)
            {
            m_SceneRenderer->Render ( commandBuffer );
            }

            // Render debug information
        if (m_DebugRenderer)
            {
            m_DebugRenderer->Render ( commandBuffer, GetViewMatrix () * GetProjectionMatrix () );
            }

            // End render pass
        vkCmdEndRenderPass ( commandBuffer );

        result = vkEndCommandBuffer ( commandBuffer );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to record command buffer" );
            }
        }

    void CEVulkanRenderer::RenderFallbackTriangle ( VkCommandBuffer commandBuffer )
        {
            // Simple fallback rendering when no scene is available
        auto pipeline = m_PipelineManager->GetPipeline ( PipelineType::StaticMesh );
        if (pipeline)
            {
            pipeline->Bind ( commandBuffer );

            // Simple triangle draw call
            vkCmdDraw ( commandBuffer, 3, 1, 0, 0 );
            }
        }
    }