// Graphics/Vulkan/Core/VulkanDevice.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <memory>

namespace CE
    {
    struct QueueFamilyIndices
        {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily;

        bool IsComplete () const {
            return graphicsFamily.has_value () && presentFamily.has_value ();
            }
        };

    class VulkanDevice
        {
        public:
            VulkanDevice ();
            ~VulkanDevice ();

            bool Initialize ( VkInstance instance, VkSurfaceKHR surface );
            void Shutdown ();

            VkPhysicalDevice GetPhysicalDevice () const { return m_PhysicalDevice; }
            VkDevice GetDevice () const { return m_Device; }
            VkQueue GetGraphicsQueue () const { return m_GraphicsQueue; }
            VkQueue GetPresentQueue () const { return m_PresentQueue; }
            VkQueue GetComputeQueue () const { return m_ComputeQueue; }
            VkQueue GetTransferQueue () const { return m_TransferQueue; }
            QueueFamilyIndices GetQueueFamilyIndices () const { return m_QueueIndices; }

            uint32_t FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties );
            VkFormat FindSupportedFormat ( const std::vector<VkFormat> & candidates,
                                           VkImageTiling tiling, VkFormatFeatureFlags features );
            VkFormat FindDepthFormat ();
            VkSampleCountFlagBits GetMaxUsableSampleCount ();

            static const char * FormatToString ( VkFormat format );

        private:
            void PickPhysicalDevice ();
            void CreateLogicalDevice ();
            QueueFamilyIndices FindQueueFamilies ( VkPhysicalDevice device );
            bool CheckDeviceExtensionSupport ( VkPhysicalDevice device );
            bool IsDeviceSuitable ( VkPhysicalDevice device );

            VkInstance m_Instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
            VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
            VkDevice m_Device = VK_NULL_HANDLE;
            VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
            VkQueue m_PresentQueue = VK_NULL_HANDLE;
            VkQueue m_ComputeQueue = VK_NULL_HANDLE;
            VkQueue m_TransferQueue = VK_NULL_HANDLE;
            QueueFamilyIndices m_QueueIndices;

            const std::vector<const char *> m_DeviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
                };
        };
    }