#include "Platform/Window/CEWindow.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEWindow::CEWindow ( const std::string & title, int width, int height )
        : Title ( title ), Width ( width ), Height ( height )
        {
        }

    CEWindow::~CEWindow ()
        {
        Shutdown ();
        }

    bool CEWindow::Initialize ()
        {
        if (!glfwInit ())
            {
            CE_CORE_ERROR ( "Failed to initialize GLFW" );
            return false;
            }
        CE_CORE_DEBUG ( "GLFW initialized" );

            // Vulkan requires GLFW_NO_API
        glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
        glfwWindowHint ( GLFW_RESIZABLE, GLFW_TRUE );

        Window = glfwCreateWindow ( Width, Height, Title.c_str (), nullptr, nullptr );
        if (!Window)
            {
            CE_CORE_ERROR ( "Failed to create GLFW window" );
            glfwTerminate ();
            return false;
            }
        CE_CORE_DEBUG ( "GLFW window created successfully" );
            // Set callbacks
        glfwSetWindowUserPointer ( Window, this );
        glfwSetFramebufferSizeCallback ( Window, FramebufferResizeCallback );

        CE_CORE_DEBUG ( "Created window: {} ({}x{})", Title, Width, Height );
        CE_CORE_DEBUG ( "Window initialized successfully" );
        return true;
        }

    void CEWindow::Shutdown ()
        {
        if (Window)
            {
            glfwDestroyWindow ( Window );
            Window = nullptr;
            }
        glfwTerminate ();
        }

    void CEWindow::PollEvents ()
        {
        glfwPollEvents ();
        }

    void CEWindow::SwapBuffers ()
        {
            // For Vulkan, we handle swapchain ourselves
        }

    void CEWindow::FramebufferResizeCallback ( GLFWwindow * window, int width, int height )
        {
        auto * ceWindow = static_cast< CEWindow * >( glfwGetWindowUserPointer ( window ) );
        if (ceWindow)
            {
            ceWindow->Width = width;
            ceWindow->Height = height;
            if (ceWindow->ResizeCallback)
                {
                ceWindow->ResizeCallback ();
                }
            }
        }
    }