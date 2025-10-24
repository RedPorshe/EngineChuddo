#include "CEVulkanSync.hpp"
#include "CEVulkanContext.hpp"
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
    {
    CEVulkanSync::CEVulkanSync ( CEVulkanContext * context )
        : Context ( context )
        {
        CE_CORE_DEBUG ( "Vulkan sync manager created" );
        }

    CEVulkanSync::~CEVulkanSync ()
        {
        Shutdown ();
        }

    bool CEVulkanSync::Initialize ()
        {
        try
            {
            CreateSyncObjects ();
            CE_CORE_DEBUG ( "Vulkan sync manager initialized successfully" );
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
        auto device = Context ? Context->GetDevice () : VK_NULL_HANDLE;

        if (device == VK_NULL_HANDLE)
            {
            CE_CORE_DEBUG ( "No valid device for sync manager shutdown" );
            return;
            }

        for (size_t i = 0; i < ImageAvailableSemaphores.Size () && i < MAX_FRAMES_IN_FLIGHT; i++)
            {
            if (RenderFinishedSemaphores[ i ] != VK_NULL_HANDLE)
                {
                vkDestroySemaphore ( device, RenderFinishedSemaphores[ i ], nullptr );
                RenderFinishedSemaphores[ i ] = VK_NULL_HANDLE;
                }

            if (ImageAvailableSemaphores[ i ] != VK_NULL_HANDLE)
                {
                vkDestroySemaphore ( device, ImageAvailableSemaphores[ i ], nullptr );
                ImageAvailableSemaphores[ i ] = VK_NULL_HANDLE;
                }

            if (InFlightFences[ i ] != VK_NULL_HANDLE)
                {
                vkDestroyFence ( device, InFlightFences[ i ], nullptr );
                InFlightFences[ i ] = VK_NULL_HANDLE;
                }
            }

        ImageAvailableSemaphores.Clear ();
        RenderFinishedSemaphores.Clear ();
        InFlightFences.Clear ();

        CE_CORE_DEBUG ( "Vulkan sync manager shutdown complete" );
        }

    void CEVulkanSync::CreateSyncObjects ()
        {
        auto device = Context->GetDevice ();

        ImageAvailableSemaphores.Resize ( MAX_FRAMES_IN_FLIGHT );
        RenderFinishedSemaphores.Resize ( MAX_FRAMES_IN_FLIGHT );
        InFlightFences.Resize ( MAX_FRAMES_IN_FLIGHT );

        VkSemaphoreCreateInfo semaphoreInfo {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Важно: создаем в signaled состоянии

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
            VkResult semaphoreResult1 = vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &ImageAvailableSemaphores[ i ] );
            VkResult semaphoreResult2 = vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &RenderFinishedSemaphores[ i ] );
            VkResult fenceResult = vkCreateFence ( device, &fenceInfo, nullptr, &InFlightFences[ i ] );

            if (semaphoreResult1 != VK_SUCCESS || semaphoreResult2 != VK_SUCCESS || fenceResult != VK_SUCCESS)
                {
                CE_CORE_ERROR ( "Failed to create sync objects for frame {}: sem1={}, sem2={}, fence={}",
                                i, static_cast< int > ( semaphoreResult1 ), static_cast< int > ( semaphoreResult2 ), static_cast< int > ( fenceResult ) );
                throw std::runtime_error ( "Failed to create synchronization objects for a frame!" );
                }

            }

        CE_CORE_DEBUG ( "Sync objects created for {} frames in flight", MAX_FRAMES_IN_FLIGHT );
        }

    bool CEVulkanSync::WaitForFrameFence ()
        {
        auto device = Context->GetDevice ();

        if (CurrentFrame >= InFlightFences.Size ())
            {
            CE_CORE_ERROR ( "Current frame index out of bounds: {} >= {}", CurrentFrame, InFlightFences.Size () );
            return false;
            }

        // Ждем завершения предыдущего кадра для этого индекса
        VkResult waitResult = vkWaitForFences ( device, 1, &InFlightFences[ CurrentFrame ], VK_TRUE, UINT64_MAX );
        if (waitResult != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to wait for fence: {}", static_cast< int >( waitResult ) );
            return false;
            }

      
        return true;
        }

    bool CEVulkanSync::ResetFrameFence ()
        {
        auto device = Context->GetDevice ();

        if (CurrentFrame >= InFlightFences.Size ())
            {
            CE_CORE_ERROR ( "Current frame index out of bounds: {} >= {}", CurrentFrame, InFlightFences.Size () );
            return false;
            }

        // Сбрасываем fence для нового кадра
        VkResult resetResult = vkResetFences ( device, 1, &InFlightFences[ CurrentFrame ] );
        if (resetResult != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to reset fence: {}", static_cast< int >( resetResult ) );
            return false;
            }

       
        return true;
        }

    bool CEVulkanSync::IsFrameFenceSignaled ()
        {
        auto device = Context->GetDevice ();

        if (CurrentFrame >= InFlightFences.Size ())
            {
            return false;
            }

        VkResult result = vkGetFenceStatus ( device, InFlightFences[ CurrentFrame ] );
        return result == VK_SUCCESS;
        }

    // Устаревший метод - для обратной совместимости
    void CEVulkanSync::WaitForFrame ()
        {
        auto device = Context->GetDevice ();

        if (CurrentFrame >= InFlightFences.Size ())
            {
            throw std::runtime_error ( "Current frame index out of bounds!" );
            }

        // Ждем завершения предыдущего кадра для этого индекса
        VkResult waitResult = vkWaitForFences ( device, 1, &InFlightFences[ CurrentFrame ], VK_TRUE, UINT64_MAX );
        if (waitResult != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to wait for fence: {}", static_cast< int >( waitResult ) );
            throw std::runtime_error ( "Failed to wait for fence!" );
            }

        // Сбрасываем fence ДО начала нового кадра
        VkResult resetResult = vkResetFences ( device, 1, &InFlightFences[ CurrentFrame ] );
        if (resetResult != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to reset fence: {}", static_cast< int >( resetResult ) );
            throw std::runtime_error ( "Failed to reset fence!" );
            }
        }

    void CEVulkanSync::SubmitFrame ()
        {
        // Этот метод может быть использован для дополнительной логики при завершении кадра
        // В текущей реализации просто логируем
       
        }

    void CEVulkanSync::AdvanceFrame ()
        {
        uint32_t previousFrame = CurrentFrame;
        CurrentFrame = ( CurrentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;       
        }
    }