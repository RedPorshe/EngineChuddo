// Runtime/Renderer/CERenderer.hpp
#pragma once
#include "Platform/Window/CEWindow.hpp"

namespace CE
    {
    class CERenderer
        {
        public:
            virtual ~CERenderer () = default;

            virtual bool Initialize ( CEWindow * window ) = 0;
            virtual void Shutdown () = 0;
            virtual void RenderFrame () = 0;
            virtual void OnWindowResized () = 0;
        };
    }