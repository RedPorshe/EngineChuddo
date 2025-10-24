// Source/App/ChudEngineApp.hpp
#pragma once
#include "Core/Application/CEApplication.hpp"

class ChudEngineApp : public CE::CEApplication
    {
    public:
        void Initialize () override;
        void Update ( float deltaTime ) override;
        void Render () override;
        void Shutdown () override;
        void SetupCamera ();
    private:
        void SetInitialized ( bool initialized ) { m_Initialized = initialized; }
        bool m_Initialized { false };
        void CreateTestScene ();
        void InitializeMeshComponents ();

        CE::Math::Vector3 CameraPosition;
        CE::Math::Vector3 CameraTarget;
        float CameraFOV;

    };