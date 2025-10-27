#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Platform/Window/CEWindow.hpp"
#include "Utils/Logger.hpp"
#include <set>

#ifdef _DEBUG
#include <iostream>
#endif

namespace CE
    {
    CEVulkanContext::CEVulkanContext ()
        : m_Instance ( VK_NULL_HANDLE )
        , m_Surface ( VK_NULL_HANDLE )
        , m_Device ( std::make_unique<VulkanDevice> () )
        {
        }

    CEVulkanContext::~CEVulkanContext ()
        {
        Shutdown ();
        }

    bool CEVulkanContext::Initialize ( CEWindow * window )
        {
        if (m_Instance != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Vulkan context already initialized" );
            return true;
            }

        try
            {
            CreateInstance ();
            CreateSurface ( window );

            if (!m_Device->Initialize ( m_Instance, m_Surface ))
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan device" );
                return false;
                }

            CE_CORE_INFO ( "Vulkan context initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan context: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanContext::Shutdown ()
        {
        if (m_Device)
            {
            m_Device->Shutdown ();
            }

        if (m_Surface != VK_NULL_HANDLE)
            {
            vkDestroySurfaceKHR ( m_Instance, m_Surface, nullptr );
            m_Surface = VK_NULL_HANDLE;
            }

        if (m_Instance != VK_NULL_HANDLE)
            {
            vkDestroyInstance ( m_Instance, nullptr );
            m_Instance = VK_NULL_HANDLE;
            }
        }

    void CEVulkanContext::CreateInstance ()
        {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "CE Application";
        appInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
        appInfo.pEngineName = "CE Engine";
        appInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions ();
        createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size () );
        createInfo.ppEnabledExtensionNames = extensions.data ();

#ifdef _DEBUG
        if (CheckValidationLayerSupport ())
            {
            createInfo.enabledLayerCount = static_cast< uint32_t >( m_ValidationLayers.size () );
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data ();
            }
        else
            {
            CE_CORE_WARN ( "Validation layers requested, but not available!" );
            createInfo.enabledLayerCount = 0;
            }
#else
        createInfo.enabledLayerCount = 0;
#endif

        VkResult result = vkCreateInstance ( &createInfo, nullptr, &m_Instance );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create Vulkan instance" );
            }

        CE_CORE_DEBUG ( "Vulkan instance created successfully" );
        }

    void CEVulkanContext::CreateSurface ( CEWindow * window )
        {
        if (!window->CreateVulkanSurface ( m_Instance, &m_Surface ))
            {
            throw std::runtime_error ( "Failed to create window surface" );
            }
        CE_CORE_DEBUG ( "Vulkan surface created successfully" );
        }

    bool CEVulkanContext::CheckValidationLayerSupport ()
        {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

        std::vector<VkLayerProperties> availableLayers ( layerCount );
        vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.data () );

        for (const char * layerName : m_ValidationLayers)
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

    std::vector<const char *> CEVulkanContext::GetRequiredExtensions ()
        {
        std::vector<const char *> extensions;

        // Get window extensions
        auto windowExtensions = CEWindow::GetRequiredVulkanExtensions ();
        extensions.insert ( extensions.end (), windowExtensions.begin (), windowExtensions.end () );

#ifdef _DEBUG
        extensions.push_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

        return extensions;
        }

    bool CEVulkanContext::IsValid () const
        {
        return m_Instance != VK_NULL_HANDLE &&
            m_Surface != VK_NULL_HANDLE &&
            m_Device &&
            m_Device->GetDevice () != VK_NULL_HANDLE;
        }
    }