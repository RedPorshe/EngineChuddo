#include "Graphics/Vulkan/Core/VulkanDevice.hpp"
#include "Utils/Logger.hpp"
#include <stdexcept>
#include <set>
#include <vector>

namespace CE
    {
    VulkanDevice::VulkanDevice ()
        {
        CE_CORE_DEBUG ( "Vulkan device created" );
        }

    VulkanDevice::~VulkanDevice ()
        {
        Shutdown ();
        }

    bool VulkanDevice::Initialize ( VkInstance instance, VkSurfaceKHR surface )
        {
        m_Instance = instance;
        m_Surface = surface;

        try
            {
            PickPhysicalDevice ();
            CreateLogicalDevice ();

            CE_CORE_DEBUG ( "Vulkan device initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan device: {}", e.what () );
                return false;
                }
        }

    void VulkanDevice::Shutdown ()
        {
        if (m_Device != VK_NULL_HANDLE)
            {
            vkDeviceWaitIdle ( m_Device );
            vkDestroyDevice ( m_Device, nullptr );
            m_Device = VK_NULL_HANDLE;
            }

            // Note: We don't own instance and surface
        m_Instance = VK_NULL_HANDLE;
        m_Surface = VK_NULL_HANDLE;
        m_PhysicalDevice = VK_NULL_HANDLE;

        CE_CORE_DEBUG ( "Vulkan device shutdown complete" );
        }

    void VulkanDevice::PickPhysicalDevice ()
        {
        CE_CORE_DEBUG ( "=== Selecting physical device ===" );

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices ( m_Instance, &deviceCount, nullptr );

        if (deviceCount == 0)
            {
            throw std::runtime_error ( "No Vulkan-capable devices found" );
            }

        std::vector<VkPhysicalDevice> devices ( deviceCount );
        vkEnumeratePhysicalDevices ( m_Instance, &deviceCount, devices.data () );

        // For now, pick the first suitable device
        for (const auto & device : devices)
            {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties ( device, &deviceProperties );

            CE_CORE_DEBUG ( "Found device: {}", deviceProperties.deviceName );

            // Basic suitability check
            QueueFamilyIndices indices = FindQueueFamilies ( device );
            if (indices.graphicsFamily.has_value ())
                {
                m_PhysicalDevice = device;
                m_QueueIndices = indices;
                CE_CORE_DEBUG ( "Selected device: {}", deviceProperties.deviceName );
                return;
                }
            }

        throw std::runtime_error ( "No suitable physical device found" );
        }

    void VulkanDevice::CreateLogicalDevice ()
        {
        CE_CORE_DEBUG ( "=== Creating logical device ===" );

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            m_QueueIndices.graphicsFamily.value ()
            };

        if (m_QueueIndices.presentFamily.has_value ())
            {
            uniqueQueueFamilies.insert ( m_QueueIndices.presentFamily.value () );
            }

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
            {
            VkDeviceQueueCreateInfo queueCreateInfo {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back ( queueCreateInfo );
            }

        VkPhysicalDeviceFeatures deviceFeatures {};
        deviceFeatures.samplerAnisotropy = VK_FALSE;
        deviceFeatures.geometryShader = VK_TRUE;

        // Convert CEArray to std::vector for Vulkan API
        std::vector<const char *> extensions;
        for (const char * ext : m_DeviceExtensions)
            {
            extensions.push_back ( ext );
            }

        VkDeviceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size () );
        createInfo.pQueueCreateInfos = queueCreateInfos.data ();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size () );
        createInfo.ppEnabledExtensionNames = extensions.data ();
        createInfo.enabledLayerCount = 0; // Device layers are deprecated

        if (vkCreateDevice ( m_PhysicalDevice, &createInfo, nullptr, &m_Device ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create logical device" );
            }

            // Get queues
        vkGetDeviceQueue ( m_Device, m_QueueIndices.graphicsFamily.value (), 0, &m_GraphicsQueue );
        if (m_QueueIndices.presentFamily.has_value ())
            {
            vkGetDeviceQueue ( m_Device, m_QueueIndices.presentFamily.value (), 0, &m_PresentQueue );
            }
        else
            {
            m_PresentQueue = m_GraphicsQueue;
            }

        CE_CORE_DEBUG ( "Logical device created successfully" );
        }

    QueueFamilyIndices VulkanDevice::FindQueueFamilies ( VkPhysicalDevice device )
        {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

        std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );
        vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

        int i = 0;
        for (const auto & queueFamily : queueFamilies)
            {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                indices.graphicsFamily = i;
                }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, m_Surface, &presentSupport );
            if (presentSupport)
                {
                indices.presentFamily = i;
                }

            if (indices.IsComplete ())
                {
                break;
                }
            i++;
            }

        return indices;
        }

    bool VulkanDevice::CheckDeviceExtensionSupport ( VkPhysicalDevice device )
        {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, nullptr );

        std::vector<VkExtensionProperties> availableExtensions ( extensionCount );
        vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, availableExtensions.data () );

        for (const char * requiredExt : m_DeviceExtensions)
            {
            bool found = false;
            for (const auto & availableExt : availableExtensions)
                {
                if (strcmp ( requiredExt, availableExt.extensionName ) == 0)
                    {
                    found = true;
                    break;
                    }
                }
            if (!found)
                {
                return false;
                }
            }

        return true;
        }

    uint32_t VulkanDevice::FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties )
        {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties ( m_PhysicalDevice, &memProperties );

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
            if (( typeFilter & ( 1 << i ) ) &&
                 ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties)
                {
                return i;
                }
            }

        throw std::runtime_error ( "Failed to find suitable memory type!" );
        }

    VkFormat VulkanDevice::FindSupportedFormat ( const CEArray<VkFormat> & candidates,
                                                 VkImageTiling tiling, VkFormatFeatureFlags features )
        {
        for (VkFormat format : candidates)
            {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties ( m_PhysicalDevice, format, &props );

            if (tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features)
                {
                return format;
                }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features)
                {
                return format;
                }
            }

        throw std::runtime_error ( "Failed to find supported format!" );
        }

    VkFormat VulkanDevice::FindDepthFormat ()
        {
        CEArray<VkFormat> candidates;
        candidates.PushBack ( VK_FORMAT_D32_SFLOAT );
        candidates.PushBack ( VK_FORMAT_D32_SFLOAT_S8_UINT );
        candidates.PushBack ( VK_FORMAT_D24_UNORM_S8_UINT );

        return FindSupportedFormat (
            candidates,
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
        }

    const char * VulkanDevice::FormatToString ( VkFormat format )
        {
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
    }