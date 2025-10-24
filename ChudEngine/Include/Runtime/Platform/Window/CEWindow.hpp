#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace CE
    {
    class CEWindow
        {
        public:
            using EventCallback = std::function<void ()>;

            CEWindow ( const std::string & title, int width, int height );
            ~CEWindow ();

            bool Initialize ();
            void Shutdown ();
            void PollEvents ();
            void SwapBuffers ();

            bool ShouldClose () const { return glfwWindowShouldClose ( Window ); }
            GLFWwindow * GetNativeWindow () const { return Window; }

            int GetWidth () const { return Width; }
            int GetHeight () const { return Height; }

            // Callbacks
            void SetResizeCallback ( EventCallback callback ) { ResizeCallback = callback; }

            

        private:
            GLFWwindow * Window = nullptr;
            std::string Title;
            int Width, Height;

            EventCallback ResizeCallback;

            static void FramebufferResizeCallback ( GLFWwindow * window, int width, int height );
        };
    }