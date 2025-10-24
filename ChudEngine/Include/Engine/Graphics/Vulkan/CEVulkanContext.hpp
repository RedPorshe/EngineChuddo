// Runtime/Renderer/Vulkan/CEVulkanContext.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "Core/Containers/CEArray.hpp"
#include <memory>
#include <optional>

namespace CE
    {
    class CEWindow;

    struct QueueFamilyIndices
        {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete () const
            {
            return graphicsFamily.has_value () && presentFamily.has_value ();
            }
        };

    class CEVulkanContext
        {
        public:
            CEVulkanContext ();
            ~CEVulkanContext ();

            bool Initialize ( CEWindow * window );
            void Shutdown ();

            // Getters
            VkInstance GetInstance () const { return Instance; }
            VkPhysicalDevice GetPhysicalDevice () const { return PhysicalDevice; }
            VkDevice GetDevice () const { return Device; }
            VkQueue GetGraphicsQueue () const { return GraphicsQueue; }
            VkQueue GetPresentQueue () const { return PresentQueue; }
            VkSurfaceKHR GetSurface () const { return Surface; }
            QueueFamilyIndices GetQueueFamilyIndices () const { return QueueIndices; }

             // ДОБАВЛЯЕМ: Хелпер для форматирования VkFormat
            static const char * FormatToString ( VkFormat format ) {
                switch (format)
                    {
                        case VK_FORMAT_UNDEFINED: return "VK_FORMAT_UNDEFINED";
                        case VK_FORMAT_B8G8R8A8_SRGB: return "VK_FORMAT_B8G8R8A8_SRGB";
                        case VK_FORMAT_R8G8B8A8_SRGB: return "VK_FORMAT_R8G8B8A8_SRGB";
                        case VK_FORMAT_D32_SFLOAT: return "VK_FORMAT_D32_SFLOAT";
                        case VK_FORMAT_D32_SFLOAT_S8_UINT: return "VK_FORMAT_D32_SFLOAT_S8_UINT";
                        case VK_FORMAT_D24_UNORM_S8_UINT: return "VK_FORMAT_D24_UNORM_S8_UINT";
                        default: return "UNKNOWN_FORMAT";
                    }
                }


            // Helper methods
            uint32_t FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties );
            VkFormat FindSupportedFormat ( const CEArray<VkFormat> & candidates, VkImageTiling tiling, VkFormatFeatureFlags features );
            VkFormat FindDepthFormat ();

        private:
            void CreateInstance ();
            void CreateSurface ( CEWindow * window );
            void PickPhysicalDevice ();
            void CreateLogicalDevice ();

            // Helper methods
            bool CheckValidationLayerSupport ();
            CEArray<const char *> GetRequiredExtensions ();
            int IsDeviceSuitable ( VkPhysicalDevice device );
            QueueFamilyIndices FindQueueFamilies ( VkPhysicalDevice device );
            bool CheckDeviceExtensionSupport ( VkPhysicalDevice device );

            // Vulkan handles
            VkInstance Instance = VK_NULL_HANDLE;
            VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
            VkDevice Device = VK_NULL_HANDLE;
            VkQueue GraphicsQueue = VK_NULL_HANDLE;
            VkQueue PresentQueue = VK_NULL_HANDLE;
            VkSurfaceKHR Surface = VK_NULL_HANDLE;

            QueueFamilyIndices QueueIndices;
        };
    }