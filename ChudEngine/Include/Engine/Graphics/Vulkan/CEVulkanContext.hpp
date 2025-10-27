#pragma once
#include <vulkan/vulkan.h>
#include "Core/Containers/CEArray.hpp"
#include "VulkanDevice.hpp"
#include <memory>

namespace CE
    {
    class CEWindow;

    class CEVulkanContext
        {
        public:
            CEVulkanContext ();
            ~CEVulkanContext ();

            bool Initialize ( CEWindow * window );
            void Shutdown ();

            // Getters
            VkInstance GetInstance () const { return m_Instance; }
            VkSurfaceKHR GetSurface () const { return m_Surface; }
            VulkanDevice * GetDevice () { return m_Device.get (); }

            // Helper methods
            static bool CheckValidationLayerSupport ();
            static CEArray<const char *> GetRequiredExtensions ();

        private:
            void CreateInstance ();
            void CreateSurface ( CEWindow * window );

            VkInstance m_Instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
            std::unique_ptr<VulkanDevice> m_Device;

            // Validation layers (debug only)
#ifdef _DEBUG
            const CEArray<const char *> m_ValidationLayers = {
                "VK_LAYER_KHRONOS_validation"
                };
#endif
        };
    }