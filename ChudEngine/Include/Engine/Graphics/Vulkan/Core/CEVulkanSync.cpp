#include "Graphics/Vulkan/Core/CEVulkanSync.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Core/CEVulkanSwapchain.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanSync::CEVulkanSync ()
        : m_Context ( nullptr )
        , m_CurrentFrame ( 0 )
        , m_MaxFramesInFlight ( 2 )
        {
        }

    CEVulkanSync::~CEVulkanSync ()
        {
        Shutdown ();
        }

    bool CEVulkanSync::Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight )
        {
        if (!context || !context->GetDevice ())
            {
            CE_CORE_ERROR ( "Invalid Vulkan context for sync manager" );
            return false;
            }

        m_Context = context;
        m_MaxFramesInFlight = maxFramesInFlight;

        try
            {
            CreateSyncObjects ();
            CE_CORE_DEBUG ( "Sync objects initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize sync objects: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanSync::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            for (size_t i = 0; i < m_ImageAvailableSemaphores.size (); i++)
                {
                if (m_ImageAvailableSemaphores[ i ] != VK_NULL_HANDLE)
                    {
                    vkDestroySemaphore ( device, m_ImageAvailableSemaphores[ i ], nullptr );
                    }
                if (m_RenderFinishedSemaphores[ i ] != VK_NULL_HANDLE)
                    {
                    vkDestroySemaphore ( device, m_RenderFinishedSemaphores[ i ], nullptr );
                    }
                if (m_InFlightFences[ i ] != VK_NULL_HANDLE)
                    {
                    vkDestroyFence ( device, m_InFlightFences[ i ], nullptr );
                    }
                }

            m_ImageAvailableSemaphores.clear ();
            m_RenderFinishedSemaphores.clear ();
            m_InFlightFences.clear ();
            }

        m_CurrentFrame = 0;
        m_Context = nullptr;
        }

    VkResult CEVulkanSync::AcquireNextImage ( CEVulkanSwapchain * swapchain, uint32_t * imageIndex )
        {
        if (!m_Context || !m_Context->GetDevice () || !swapchain)
            {
            CE_CORE_ERROR ( "Invalid state for image acquisition" );
            return VK_ERROR_INITIALIZATION_FAILED;
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();

        // Wait for the fence to ensure the previous frame using this index has finished
        vkWaitForFences ( device, 1, &m_InFlightFences[ m_CurrentFrame ], VK_TRUE, UINT64_MAX );

        // Acquire the next image from the swapchain
        VkResult result = swapchain->AcquireNextImage (
            m_ImageAvailableSemaphores[ m_CurrentFrame ],
            imageIndex
        );

        return result;
        }

    bool CEVulkanSync::SubmitFrame ( VkCommandBuffer commandBuffer, CEVulkanSwapchain * swapchain, uint32_t imageIndex )
        {
        if (!m_Context || !m_Context->GetDevice () || !swapchain)
            {
            CE_CORE_ERROR ( "Invalid state for frame submission" );
            return false;
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();

        // Reset the fence for the current frame
        vkResetFences ( device, 1, &m_InFlightFences[ m_CurrentFrame ] );

        // Submit the command buffer
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores [] = { m_ImageAvailableSemaphores[ m_CurrentFrame ] };
        VkPipelineStageFlags waitStages [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores [] = { m_RenderFinishedSemaphores[ m_CurrentFrame ] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult result = vkQueueSubmit (
            m_Context->GetDevice ()->GetGraphicsQueue (),
            1, &submitInfo,
            m_InFlightFences[ m_CurrentFrame ]
        );

        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to submit draw command buffer" );
            return false;
            }

            // Present the frame
        result = swapchain->Present ( imageIndex, m_RenderFinishedSemaphores[ m_CurrentFrame ] );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to present swap chain image" );
            return false;
            }

        return true;
        }

    void CEVulkanSync::AdvanceFrame ()
        {
        m_CurrentFrame = ( m_CurrentFrame + 1 ) % m_MaxFramesInFlight;
        }

    void CEVulkanSync::CreateSyncObjects ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for sync object creation" );
            }

        VkDevice device = m_Context->GetDevice ()->GetDevice ();

        m_ImageAvailableSemaphores.resize ( m_MaxFramesInFlight );
        m_RenderFinishedSemaphores.resize ( m_MaxFramesInFlight );
        m_InFlightFences.resize ( m_MaxFramesInFlight );

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't wait forever

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
            {
            VkResult result = vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[ i ] );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create image available semaphore" );
                }

            result = vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[ i ] );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create render finished semaphore" );
                }

            result = vkCreateFence ( device, &fenceInfo, nullptr, &m_InFlightFences[ i ] );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create in-flight fence" );
                }
            }

        CE_CORE_DEBUG ( "Created sync objects for {} frames in flight", m_MaxFramesInFlight );
        }
    }