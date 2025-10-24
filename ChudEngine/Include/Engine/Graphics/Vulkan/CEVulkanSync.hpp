// Runtime/Renderer/Vulkan/CEVulkanSync.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "Core/Containers/CEArray.hpp"

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanSync
        {
        public:
            static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

            CEVulkanSync ( CEVulkanContext * context );
            ~CEVulkanSync ();

            bool Initialize ();
            void Shutdown ();

            // Основные методы синхронизации
            bool WaitForFrameFence ();      // Только ожидание fence
            bool ResetFrameFence ();        // Только сброс fence
            bool IsFrameFenceSignaled ();   // Проверка состояния fence

            // Устаревший метод - для обратной совместимости
            void WaitForFrame ();

            void SubmitFrame ();

            VkSemaphore GetImageAvailableSemaphore () const { return ImageAvailableSemaphores[ CurrentFrame ]; }
            VkSemaphore GetRenderFinishedSemaphore () const { return RenderFinishedSemaphores[ CurrentFrame ]; }
            VkFence GetInFlightFence () const { return InFlightFences[ CurrentFrame ]; }
            uint32_t GetCurrentFrame () const { return CurrentFrame; }

            void AdvanceFrame ();

        private:
            void CreateSyncObjects ();

            CEVulkanContext * Context = nullptr;
            CEArray<VkSemaphore> ImageAvailableSemaphores;
            CEArray<VkSemaphore> RenderFinishedSemaphores;
            CEArray<VkFence> InFlightFences;
            uint32_t CurrentFrame = 0;
        };
    }