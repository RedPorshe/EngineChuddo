// Runtime/Core/Application/CEApplication.cpp
#include "Core/Application/CEApplication.hpp"
#include "Core/Logger.h"

namespace CE
    {
    CEApplication::CEApplication ()
        {
        CE_CORE_DEBUG ( "CEApplication created" );
        }

    CEApplication::~CEApplication ()
        {
       // Shutdown ();
        }

    void CEApplication::Run ()
        {
        CE_CORE_DEBUG ( "CEApplication::Run() started" );

        Initialize ();

        if (!Window || !Renderer)
            {
            CE_CORE_ERROR ( "Application not properly initialized" );
            CE_CORE_ERROR ( "Window: {}", Window ? "OK" : "NULL" );
            CE_CORE_ERROR ( "Renderer: {}", Renderer ? "OK" : "NULL" );
            return;
            }

        IsRunning = true;
        LastFrameTime = glfwGetTime ();

        CE_CORE_DEBUG ( "Starting application main loop" );

        while (IsRunning && !Window->ShouldClose ())
            {
                // Calculate delta time
            double currentTime = glfwGetTime ();
            DeltaTime = static_cast< float >( currentTime - LastFrameTime );
            LastFrameTime = currentTime;

            // Update
            Update ( DeltaTime );

            // Render
            Render ();
            
            // Poll events
            Window->PollEvents ();
           
            }
        Shutdown ();
        CE_CORE_DEBUG ( "Application main loop ended" );
        }

    void CEApplication::Quit ()
        {
        IsRunning = false;
        }

    void CEApplication::Shutdown ()
        {
        CE_CORE_DEBUG ( "CEApplication shutdown started" );

        if (World)
            {
            World->Destroy ();
            World.reset ();
            }

        if (Renderer)
            {
            Renderer->Shutdown ();
            Renderer.reset ();
            }

        if (Window)
            {
            Window->Shutdown ();
            Window.reset ();
            }

        CE_CORE_DEBUG ( "CEApplication shutdown complete" );
        }
    }