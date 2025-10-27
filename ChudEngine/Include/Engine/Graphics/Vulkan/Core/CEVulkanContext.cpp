#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Utils/Logger.hpp"
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
            m_Device->Initialize (m_Instance,m_Surface);
           

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
        m_Device->Shutdown ();

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

        if (vkCreateInstance ( &createInfo, nullptr, &m_Instance ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create Vulkan instance" );
            }

        CE_CORE_DEBUG ( "Vulkan instance created" );
        }

    void CEVulkanContext::CreateSurface ( CEWindow * window )
        {
        if (glfwCreateWindowSurface ( m_Instance, window->GetNativeWindow (), nullptr, &m_Surface ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create window surface" );
            }
        CE_CORE_DEBUG ( "Vulkan surface created" );
        }
 

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
           
    }