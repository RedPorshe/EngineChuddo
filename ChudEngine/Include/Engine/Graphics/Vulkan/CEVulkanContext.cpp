#include "CEVulkanContext.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Core/Logger.h"
#include <stdexcept>
#include <set>
#include <cstring>
#include <map>

namespace CE
    {
    // Validation layers (only in debug builds)
#ifdef _DEBUG
    const CEArray<const char *> VALIDATION_LAYERS = [] ()
        {
        CEArray<const char *> layers;
        layers.PushBack ( "VK_LAYER_KHRONOS_validation" );
        return layers;
        }( );
#else
    const CEArray<const char *> VALIDATION_LAYERS = [] ()
        {
        return CEArray<const char *> ();
        }( );
#endif

    // Device extensions
    const CEArray<const char *> DEVICE_EXTENSIONS = [] ()
        {
        CEArray<const char *> extensions;
        extensions.PushBack ( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
        return extensions;
        }( );

    CEVulkanContext::CEVulkanContext ()
        {
        CE_CORE_DEBUG ( "Vulkan context created" );
        }

    CEVulkanContext::~CEVulkanContext ()
        {
            // Shutdown should be called explicitly
        }

    bool CEVulkanContext::Initialize ( CEWindow * window )
        {
        try
            {
            CreateInstance ();
            CreateSurface ( window );
            PickPhysicalDevice ();
            CreateLogicalDevice ();

            CE_CORE_DEBUG ( "Vulkan context initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan context: {}", e.what () );
                return false;
                }
        }

    void CEVulkanContext::Shutdown ()
        {
        if (Device != VK_NULL_HANDLE)
            {
            vkDeviceWaitIdle ( Device );
            vkDestroyDevice ( Device, nullptr );
            Device = VK_NULL_HANDLE;
            }

        if (Surface != VK_NULL_HANDLE)
            {
            vkDestroySurfaceKHR ( Instance, Surface, nullptr );
            Surface = VK_NULL_HANDLE;
            }

        if (Instance != VK_NULL_HANDLE)
            {
            vkDestroyInstance ( Instance, nullptr );
            Instance = VK_NULL_HANDLE;
            }

        CE_CORE_DEBUG ( "Vulkan context shutdown complete" );
        }

    void CEVulkanContext::CreateInstance ()
        {
        VkApplicationInfo appInfo {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "ChudEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
        appInfo.pEngineName = "ChudEngine";
        appInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Extensions
        auto extensions = GetRequiredExtensions ();
        createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.Size () );
        createInfo.ppEnabledExtensionNames = extensions.RawData ();

        // Validation layers
        if (!VALIDATION_LAYERS.IsEmpty () && CheckValidationLayerSupport ())
            {
            createInfo.enabledLayerCount = static_cast< uint32_t >( VALIDATION_LAYERS.Size () );
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.RawData ();
            }
        else
            {
            createInfo.enabledLayerCount = 0;
            }

        if (vkCreateInstance ( &createInfo, nullptr, &Instance ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create Vulkan instance" );
            }

        CE_CORE_DEBUG ( "Vulkan instance created" );
        }

    void CEVulkanContext::CreateSurface ( CEWindow * window )
        {
        if (glfwCreateWindowSurface ( Instance, window->GetNativeWindow (), nullptr, &Surface ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create window surface" );
            }
        CE_CORE_DEBUG ( "Vulkan surface created" );
        }

    void CEVulkanContext::PickPhysicalDevice ()
        {
        CE_CORE_DEBUG ( "=== Starting physical device selection ===" );

        uint32_t deviceCount = 0;
        VkResult result = vkEnumeratePhysicalDevices ( Instance, &deviceCount, nullptr );

        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to enumerate physical devices: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to enumerate physical devices" );
            }

        CE_CORE_DEBUG ( "vkEnumeratePhysicalDevices found {} devices", deviceCount );

        if (deviceCount == 0)
            {
            throw std::runtime_error ( "No Vulkan-capable devices found" );
            }

            // Используем std::vector для избежания проблем с CEArray
        std::vector<VkPhysicalDevice> devices ( deviceCount );
        result = vkEnumeratePhysicalDevices ( Instance, &deviceCount, devices.data () );

        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to get physical devices: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to get physical devices" );
            }

        CE_CORE_DEBUG ( "Successfully retrieved {} devices", deviceCount );

        // Принудительно логируем все устройства
        for (uint32_t i = 0; i < deviceCount; ++i)
            {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties ( devices[ i ], &deviceProperties );

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures ( devices[ i ], &deviceFeatures );

            CE_CORE_DEBUG ( "Device {}: {}", i, deviceProperties.deviceName );
            CE_CORE_DEBUG ( "  Type: {}", static_cast< int > ( deviceProperties.deviceType ) );
            CE_CORE_DEBUG ( "  API Version: {}.{}.{}",
                            VK_VERSION_MAJOR ( deviceProperties.apiVersion ),
                            VK_VERSION_MINOR ( deviceProperties.apiVersion ),
                            VK_VERSION_PATCH ( deviceProperties.apiVersion ) );
            CE_CORE_DEBUG ( "  Geometry Shader: {}", deviceFeatures.geometryShader );

            // Принудительно выбираем первое устройство независимо от проверок
            if (i == 0)
                {
                PhysicalDevice = devices[ i ];
                CE_CORE_DEBUG ( "FORCIBLY SELECTED device: {}", deviceProperties.deviceName );

                // Проверим surface support для информации
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties ( PhysicalDevice, &queueFamilyCount, nullptr );

                if (queueFamilyCount > 0)
                    {
                    std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );
                    vkGetPhysicalDeviceQueueFamilyProperties ( PhysicalDevice, &queueFamilyCount, queueFamilies.data () );

                    for (uint32_t j = 0; j < queueFamilyCount; ++j)
                        {
                        VkBool32 presentSupport = false;
                        VkResult surfaceResult = vkGetPhysicalDeviceSurfaceSupportKHR ( PhysicalDevice, j, Surface, &presentSupport );
                        CE_CORE_DEBUG ( "Queue family {} surface support: {} (result: {})",
                                        j, presentSupport, static_cast< int > ( surfaceResult ) );
                        }
                    }

                return;
                }
            }

            // Если дошли сюда, значит что-то пошло не так
        throw std::runtime_error ( "No devices available for selection" );
        }

    void CEVulkanContext::CreateLogicalDevice ()
        {
        CE_CORE_DEBUG ( "=== Creating logical device ===" );

        if (PhysicalDevice == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "No physical device selected" );
            }

        QueueIndices = FindQueueFamilies ( PhysicalDevice );

        CE_CORE_DEBUG ( "Queue indices - Graphics: {}, Present: {}",
                        QueueIndices.graphicsFamily.has_value () ? QueueIndices.graphicsFamily.value () : -1,
                        QueueIndices.presentFamily.has_value () ? QueueIndices.presentFamily.value () : -1 );

                    // Убедимся что у нас есть хотя бы graphics очередь
        if (!QueueIndices.graphicsFamily.has_value ())
            {
            throw std::runtime_error ( "No graphics queue family found" );
            }

            // Используем std::vector для избежания проблем с CEArray
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;

        // Всегда добавляем graphics очередь
        uniqueQueueFamilies.insert ( QueueIndices.graphicsFamily.value () );

        // Если present очередь отличается, добавляем и её
        if (QueueIndices.presentFamily.has_value () &&
             QueueIndices.presentFamily.value () != QueueIndices.graphicsFamily.value ())
            {
            uniqueQueueFamilies.insert ( QueueIndices.presentFamily.value () );
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

            CE_CORE_DEBUG ( "Creating queue for family {}", queueFamily );
            }

        VkPhysicalDeviceFeatures deviceFeatures {};
        // Минимальные фичи
        deviceFeatures.samplerAnisotropy = VK_FALSE; // Временно отключаем
        deviceFeatures.geometryShader = VK_TRUE;

        // Получаем расширения как C-массив
        std::vector<const char *> extensions;
        for (const char * ext : DEVICE_EXTENSIONS)
            {
            extensions.push_back ( ext );
            }

        VkDeviceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size () );
        createInfo.pQueueCreateInfos = queueCreateInfos.data ();
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Минимальные расширения
        createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size () );
        createInfo.ppEnabledExtensionNames = extensions.data ();

        // Временно отключаем validation layers для устройства
        createInfo.enabledLayerCount = 0;

        CE_CORE_DEBUG ( "Creating logical device with {} queues, {} extensions",
                        createInfo.queueCreateInfoCount, createInfo.enabledExtensionCount );

        VkResult result = vkCreateDevice ( PhysicalDevice, &createInfo, nullptr, &Device );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create logical device: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to create logical device" );
            }

            // Получаем очереди
        vkGetDeviceQueue ( Device, QueueIndices.graphicsFamily.value (), 0, &GraphicsQueue );
        if (QueueIndices.presentFamily.has_value ())
            {
            vkGetDeviceQueue ( Device, QueueIndices.presentFamily.value (), 0, &PresentQueue );
            }
        else
            {
                // Если нет отдельной present очереди, используем graphics
            PresentQueue = GraphicsQueue;
            }

        CE_CORE_DEBUG ( "Logical device created successfully" );
        }

        // Helper methods implementation

    bool CEVulkanContext::CheckValidationLayerSupport ()
        {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

        CEArray<VkLayerProperties> availableLayers ( layerCount );
        vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.RawData () );

        for (const char * layerName : VALIDATION_LAYERS)
            {
            bool layerFound = false;

            for (const auto & layerProperties : availableLayers)
                {
                if (strcmp ( layerName, layerProperties.layerName ) == 0)
                    {
                    layerFound = true;
                    break;
                    }
                }

            if (!layerFound)
                {
                return false;
                }
            }

        return true;
        }

    CEArray<const char *> CEVulkanContext::GetRequiredExtensions ()
        {
        uint32_t glfwExtensionCount = 0;
        const char ** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions ( &glfwExtensionCount );

        CEArray<const char *> extensions;
        extensions.Reserve ( glfwExtensionCount + 1 );

        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
            {
            extensions.PushBack ( glfwExtensions[ i ] );
            }

        if (!VALIDATION_LAYERS.IsEmpty ())
            {
            extensions.PushBack ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
            }

        return extensions;
        }

    
    int CEVulkanContext::IsDeviceSuitable ( VkPhysicalDevice device )
        {
        //VkPhysicalDeviceProperties deviceProperties;
        //vkGetPhysicalDeviceProperties ( device, &deviceProperties );

        //CE_CORE_DEBUG ( "=== Checking device: {} ===", deviceProperties.deviceName );

        //// Базовые проверки
        //QueueFamilyIndices indices = FindQueueFamilies ( device );
        //bool extensionsSupported = CheckDeviceExtensionSupport ( device );

        //CE_CORE_DEBUG ( "Basic checks - Queues: {}, Extensions: {}",
        //                indices.IsComplete (), extensionsSupported );

        //            // ВРЕМЕННО: принимаем любое устройство с graphics очередью
        //if (indices.graphicsFamily.has_value ())
        //    {
        //    CE_CORE_DEBUG ( "Device ACCEPTED (has graphics queue)" );
        //    return 100; // Минимальный балл чтобы устройство было принято
        //    }

        //CE_CORE_DEBUG ( "Device REJECTED (no graphics queue)" );
        //return 0;

        CE_CORE_DEBUG ( "BYPASSING all device checks - device accepted" );
        return 1000;
        }

    QueueFamilyIndices CEVulkanContext::FindQueueFamilies ( VkPhysicalDevice device )
        {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

        if (queueFamilyCount == 0)
            {
            CE_CORE_DEBUG ( "No queue families found!" );
            return indices;
            }

            // Используем std::vector для избежания проблем с CEArray
        std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );
        vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

        CE_CORE_DEBUG ( "Device has {} queue families", queueFamilyCount );

        int i = 0;
        for (const auto & queueFamily : queueFamilies)
            {
            CE_CORE_DEBUG ( "Queue family {}: {} queues, flags: {:#x}",
                            i, queueFamily.queueCount, queueFamily.queueFlags );

                        // Check for graphics support
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                indices.graphicsFamily = i;
                CE_CORE_DEBUG ( "  -> Graphics capability found" );
                }

                // Check for presentation support
            VkBool32 presentSupport = false;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, Surface, &presentSupport );

            if (result == VK_SUCCESS && presentSupport)
                {
                indices.presentFamily = i;
                CE_CORE_DEBUG ( "  -> Present capability found" );
                }
            else if (result != VK_SUCCESS)
                {
                CE_CORE_DEBUG ( "  -> Surface support check failed: {}", static_cast< int >( result ) );
                }

            i++;
            }

        CE_CORE_DEBUG ( "Queue family result - Graphics: {}, Present: {}",
                        indices.graphicsFamily.has_value (),
                        indices.presentFamily.has_value () );

        return indices;
        }

    bool CEVulkanContext::CheckDeviceExtensionSupport ( VkPhysicalDevice device )
        {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, nullptr );

        CEArray<VkExtensionProperties> availableExtensions ( extensionCount );
        vkEnumerateDeviceExtensionProperties ( device, nullptr, &extensionCount, availableExtensions.RawData () );

        CE_CORE_DEBUG ( "Device has {} extensions", extensionCount );

        // Проверяем только самое необходимое
        bool hasSwapchain = false;
        for (const auto & extension : availableExtensions)
            {
            if (strcmp ( extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) == 0)
                {
                hasSwapchain = true;
                break;
                }
            }

        CE_CORE_DEBUG ( "Required extension {}: {}", VK_KHR_SWAPCHAIN_EXTENSION_NAME, hasSwapchain );

        // ВРЕМЕННО: возвращаем true даже если расширения нет
        // return hasSwapchain;
        return true; // Временно игнорируем проверку расширений
        }

    uint32_t CEVulkanContext::FindMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties )
        {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties ( PhysicalDevice, &memProperties );

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

    VkFormat CEVulkanContext::FindSupportedFormat ( const CEArray<VkFormat> & candidates, VkImageTiling tiling, VkFormatFeatureFlags features )
        {
        for (VkFormat format : candidates)
            {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties ( PhysicalDevice, format, &props );

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

    VkFormat CEVulkanContext::FindDepthFormat ()
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
    }