#include "Graphics/Vulkan/Core/VulkanDevice.hpp"
#include "Utils/Logger.hpp"
#include <set>
#include <stdexcept>

namespace CE
    {
    VulkanDevice::VulkanDevice ()
        : m_Instance ( VK_NULL_HANDLE )
        , m_Surface ( VK_NULL_HANDLE )
        , m_PhysicalDevice ( VK_NULL_HANDLE )
        , m_Device ( VK_NULL_HANDLE )
        , m_GraphicsQueue ( VK_NULL_HANDLE )
        , m_PresentQueue ( VK_NULL_HANDLE )
        , m_ComputeQueue ( VK_NULL_HANDLE )
        , m_TransferQueue ( VK_NULL_HANDLE )
        {
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
            CE_CORE_INFO ( "Vulkan device initialized successfully" );
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

        m_PhysicalDevice = VK_NULL_HANDLE;
        m_GraphicsQueue = VK_NULL_HANDLE;
        m_PresentQueue = VK_NULL_HANDLE;
        m_ComputeQueue = VK_NULL_HANDLE;
        m_TransferQueue = VK_NULL_HANDLE;
        }

    void VulkanDevice::PickPhysicalDevice ()
        {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices ( m_Instance, &deviceCount, nullptr );

        if (deviceCount == 0)
            {
            throw std::runtime_error ( "Failed to find GPUs with Vulkan support" );
            }

        std::vector<VkPhysicalDevice> devices ( deviceCount );
        vkEnumeratePhysicalDevices ( m_Instance, &deviceCount, devices.data () );

        for (const auto & device : devices)
            {
            if (IsDeviceSuitable ( device ))
                {
                m_PhysicalDevice = device;
                break;
                }
            }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Failed to find a suitable GPU" );
            }

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties ( m_PhysicalDevice, &deviceProperties );
        CE_CORE_INFO ( "Selected physical device: {}", deviceProperties.deviceName );
        }

    void VulkanDevice::CreateLogicalDevice ()
        {
        m_QueueIndices = FindQueueFamilies ( m_PhysicalDevice );

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            m_QueueIndices.graphicsFamily.value (),
            m_QueueIndices.presentFamily.value ()
            };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
            {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back ( queueCreateInfo );
            }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.fillModeNonSolid = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size () );
        createInfo.pQueueCreateInfos = queueCreateInfos.data ();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast< uint32_t >( m_DeviceExtensions.size () );
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data ();

#ifdef _DEBUG
        createInfo.enabledLayerCount = static_cast< uint32_t >( m_ValidationLayers.size () );
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data ();
#else
        createInfo.enabledLayerCount = 0;
#endif

        VkResult result = vkCreateDevice ( m_PhysicalDevice, &createInfo, nullptr, &m_Device );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create logical device" );
            }

        vkGetDeviceQueue ( m_Device, m_QueueIndices.graphicsFamily.value (), 0, &m_GraphicsQueue );
        vkGetDeviceQueue ( m_Device, m_QueueIndices.presentFamily.value (), 0, &m_PresentQueue );

        if (m_QueueIndices.computeFamily.has_value ())
            {
            vkGetDeviceQueue ( m_Device, m_QueueIndices.computeFamily.value (), 0, &m_ComputeQueue );
            }

        if (m_QueueIndices.transferFamily.has_value ())
            {
            vkGetDeviceQueue ( m_Device, m_QueueIndices.transferFamily.value (), 0, &m_TransferQueue );
            }
        else
            {
            m_TransferQueue = m_GraphicsQueue;
            }
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
            if (queueFamily.queueCount > 0)
                {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    {
                    indices.graphicsFamily = i;
                    }

                if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    {
                    indices.computeFamily = i;
                    }

                if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    {
                    indices.transferFamily = i;
                    }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, m_Surface, &presentSupport );
                if (presentSupport)
                    {
                    indices.presentFamily = i;
                    }
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

        std::set<std::string> requiredExtensions ( m_DeviceExtensions.begin (), m_DeviceExtensions.end () );

        for (const auto & extension : availableExtensions)
            {
            requiredExtensions.erase ( extension.extensionName );
            }

        return requiredExtensions.empty ();
        }

    bool VulkanDevice::IsDeviceSuitable ( VkPhysicalDevice device )
        {
        QueueFamilyIndices indices = FindQueueFamilies ( device );

        bool extensionsSupported = CheckDeviceExtensionSupport ( device );

        bool swapChainAdequate = false;
        if (extensionsSupported)
            {
// We'll check swapchain support later
            swapChainAdequate = true;
            }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures ( device, &supportedFeatures );

        return indices.IsComplete () && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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

        throw std::runtime_error ( "Failed to find suitable memory type" );
        }

    VkFormat VulkanDevice::FindSupportedFormat ( const std::vector<VkFormat> & candidates,
                                                 VkImageTiling tiling,
                                                 VkFormatFeatureFlags features )
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

        throw std::runtime_error ( "Failed to find supported format" );
        }

    VkFormat VulkanDevice::FindDepthFormat ()
        {
        return FindSupportedFormat (
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
        }

    VkSampleCountFlagBits VulkanDevice::GetMaxUsableSampleCount ()
        {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties ( m_PhysicalDevice, &physicalDeviceProperties );

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
            physicalDeviceProperties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
        }

    const char * VulkanDevice::FormatToString ( VkFormat format )
        {
        switch (format)
            {
                case VK_FORMAT_UNDEFINED: return "VK_FORMAT_UNDEFINED";
                case VK_FORMAT_R8G8B8A8_SRGB: return "VK_FORMAT_R8G8B8A8_SRGB";
                case VK_FORMAT_B8G8R8A8_SRGB: return "VK_FORMAT_B8G8R8A8_SRGB";
                case VK_FORMAT_D32_SFLOAT: return "VK_FORMAT_D32_SFLOAT";
                case VK_FORMAT_D32_SFLOAT_S8_UINT: return "VK_FORMAT_D32_SFLOAT_S8_UINT";
                case VK_FORMAT_D24_UNORM_S8_UINT: return "VK_FORMAT_D24_UNORM_S8_UINT";
                default: return "UNKNOWN_FORMAT";
            }
        }
    }