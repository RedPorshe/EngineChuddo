#pragma once
#include <memory>
#include "Platform/Window/CEWindow.hpp"
#include "Core/CEObject/CEWorld.hpp"

#include "Graphics/Vulkan/CEVulkanRenderer.hpp"

namespace CE
    {


    class CEApplication
        {
        public:
            CEApplication ();
            virtual ~CEApplication ();

            void Run ();
            void Quit ();

            virtual void Initialize () = 0;
            virtual void Update ( float deltaTime ) = 0;
            virtual void Render () = 0;
            virtual void Shutdown () = 0;

            CEWindow * GetWindow () const { return Window.get (); }
            CERenderer * GetRenderer () const { return Renderer.get (); }
            CEWorld * GetWorld () const { return World.get (); }
            
        protected:
            std::unique_ptr<CEWindow> Window;
            std::unique_ptr<CERenderer> Renderer; 
            std::unique_ptr<CEWorld> World;

        private:
           
            bool IsRunning = false;
            float DeltaTime = 0.0f;
            double LastFrameTime = 0.0;
        };
    }