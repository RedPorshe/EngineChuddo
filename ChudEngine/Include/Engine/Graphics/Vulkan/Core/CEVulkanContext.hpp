// Graphics/Vulkan/Core/CEVulkanContext.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace CE
    {
    class CEWindow;
    class VulkanDevice;

    class CEVulkanContext
        {
        public:
            CEVulkanContext ();
            ~CEVulkanContext ();

            bool Initialize ( CEWindow * window );
            void Shutdown ();

            VkInstance GetInstance () const { return m_Instance; }
            VkSurfaceKHR GetSurface () const { return m_Surface; }
            VulkanDevice * GetDevice () { return m_Device.get (); }

            static bool CheckValidationLayerSupport ();
            static std::vector<const char *> GetRequiredExtensions ();
            bool IsValid () const;

            CEVulkanContext ( const CEVulkanContext & ) = delete;
            CEVulkanContext & operator=( const CEVulkanContext & ) = delete;

        private:
            void CreateInstance ();
            void CreateSurface ( CEWindow * window );

            VkInstance m_Instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
            std::unique_ptr<VulkanDevice> m_Device;

#ifdef _DEBUG
            const std::vector<const char *> m_ValidationLayers = {
                "VK_LAYER_KHRONOS_validation"
                };
#endif
        };
    }