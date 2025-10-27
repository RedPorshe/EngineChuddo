// Runtime/Renderer/Vulkan/CEVulkanSync.cpp
#include "CEVulkanSync.hpp"
#include "CEVulkanContext.hpp"
#include "CEVulkanSwapchain.hpp"
#include "CEVulkanCommandBuffer.hpp"
#include "Core/Logger.h"
#include <stdexcept>


namespace CE
    {
    CEVulkanSync::CEVulkanSync ()
        {
        CE_CORE_DEBUG ( "Vulkan sync manager created" );
        }

    CEVulkanSync::~CEVulkanSync ()
        {
        Shutdown ();
        }

    bool CEVulkanSync::Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight )
        {
        m_Context = context;
        m_MaxFramesInFlight = maxFramesInFlight;

        try
            {
            CreateSyncObjects ();
            CE_CORE_DEBUG ( "Vulkan sync manager initialized successfully for {} frames", maxFramesInFlight );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan sync manager: {}", e.what () );
                return false;
                }
        }

    void CEVulkanSync::Shutdown ()
        {
        auto device = m_Context ? m_Context->GetDevice () : VK_NULL_HANDLE;
        if (!device) return;

        for (size_t i = 0; i < m_InFlightFences.Size (); i++)
            {
            if (m_RenderFinishedSemaphores[ i ] != VK_NULL_HANDLE)
                {
                vkDestroySemaphore ( device, m_RenderFinishedSemaphores[ i ], nullptr );
                }
            if (m_ImageAvailableSemaphores[ i ] != VK_NULL_HANDLE)
                {
                vkDestroySemaphore ( device, m_ImageAvailableSemaphores[ i ], nullptr );
                }
            if (m_InFlightFences[ i ] != VK_NULL_HANDLE)
                {
                vkDestroyFence ( device, m_InFlightFences[ i ], nullptr );
                }
            }

        m_ImageAvailableSemaphores.Clear ();
        m_RenderFinishedSemaphores.Clear ();
        m_InFlightFences.Clear ();

        CE_CORE_DEBUG ( "Vulkan sync manager shutdown complete" );
        }

    void CEVulkanSync::CreateSyncObjects ()
        {
        auto device = m_Context->GetDevice ();

        m_ImageAvailableSemaphores.Resize ( m_MaxFramesInFlight );
        m_RenderFinishedSemaphores.Resize ( m_MaxFramesInFlight );
        m_InFlightFences.Resize ( m_MaxFramesInFlight );

        VkSemaphoreCreateInfo semaphoreInfo {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
            {
            if (vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[ i ] ) != VK_SUCCESS ||
                 vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[ i ] ) != VK_SUCCESS ||
                 vkCreateFence ( device, &fenceInfo, nullptr, &m_InFlightFences[ i ] ) != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to create synchronization objects for a frame!" );
                }
            }

        CE_CORE_DEBUG ( "Sync objects created for {} frames in flight", m_MaxFramesInFlight );
        }

    VkResult CEVulkanSync::AcquireNextImage ( CEVulkanSwapchain * swapchain, uint32_t * imageIndex )
        {
        auto device = m_Context->GetDevice ();

        // Wait for the frame to be finished
        vkWaitForFences ( device, 1, &m_InFlightFences[ m_CurrentFrame ], VK_TRUE, UINT64_MAX );

        // Acquire the next image
        return swapchain->AcquireNextImage ( m_ImageAvailableSemaphores[ m_CurrentFrame ], imageIndex );
        }

    bool CEVulkanSync::SubmitFrame ( CEVulkanCommandBuffer * commandBuffer, CEVulkanSwapchain * swapchain, uint32_t imageIndex )
        {
        auto device = m_Context->GetDevice ();
        auto graphicsQueue = m_Context->GetGraphicsQueue ();

        // Reset fence before submitting
        vkResetFences ( device, 1, &m_InFlightFences[ m_CurrentFrame ] );

        // Submit command buffer
        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores [] = { m_ImageAvailableSemaphores[ m_CurrentFrame ] };
        VkPipelineStageFlags waitStages [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkCommandBuffer cmdBuffer = commandBuffer->GetCurrent ();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        VkSemaphore signalSemaphores [] = { m_RenderFinishedSemaphores[ m_CurrentFrame ] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit ( graphicsQueue, 1, &submitInfo, m_InFlightFences[ m_CurrentFrame ] ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to submit draw command buffer" );
            return false;
            }

        return true;
        }

    void CEVulkanSync::AdvanceFrame ()
        {
        m_CurrentFrame = ( m_CurrentFrame + 1 ) % m_MaxFramesInFlight;
        }
    }