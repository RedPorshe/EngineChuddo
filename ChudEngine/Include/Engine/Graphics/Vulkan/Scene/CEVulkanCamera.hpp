// Graphics/Vulkan/Scene/CEVulkanCamera.hpp
#pragma once
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

namespace CE
    {
    class CEVulkanCamera
        {
        public:
            CEVulkanCamera ();
            ~CEVulkanCamera () = default;

            void SetPosition ( const Math::Vector3 & position ) { m_Position = position; m_ViewDirty = true; }
            void SetTarget ( const Math::Vector3 & target ) { m_Target = target; m_ViewDirty = true; }
            void SetUpVector ( const Math::Vector3 & up ) { m_Up = up; m_ViewDirty = true; }
            void SetFOV ( float fov ) { m_FOV = fov; m_ProjectionDirty = true; }
            void SetAspectRatio ( float aspectRatio ) { m_AspectRatio = aspectRatio; m_ProjectionDirty = true; }
            void SetNearFar ( float nearPlane, float farPlane ) {
                m_NearPlane = nearPlane; m_FarPlane = farPlane; m_ProjectionDirty = true;
                }

            void LookAt ( const Math::Vector3 & position, const Math::Vector3 & target, const Math::Vector3 & up );
            void SetPerspective ( float fov, float aspectRatio, float nearPlane, float farPlane );
            void SetOrthographic ( float left, float right, float bottom, float top, float nearPlane, float farPlane );

            const Math::Matrix4 & GetViewMatrix ();
            const Math::Matrix4 & GetProjectionMatrix ();
            Math::Matrix4 GetViewProjectionMatrix ();
            Math::Vector3 GetForward () const;
            Math::Vector3 GetRight () const;
            Math::Vector3 GetUp () const;

            const Math::Vector3 & GetPosition () const { return m_Position; }
            const Math::Vector3 & GetTarget () const { return m_Target; }
            const Math::Vector3 & GetUpVector () const { return m_Up; }
            float GetFOV () const { return m_FOV; }
            float GetAspectRatio () const { return m_AspectRatio; }
            float GetNearPlane () const { return m_NearPlane; }
            float GetFarPlane () const { return m_FarPlane; }

            bool IsViewDirty () const { return m_ViewDirty; }
            bool IsProjectionDirty () const { return m_ProjectionDirty; }
            void MarkClean () { m_ViewDirty = m_ProjectionDirty = false; }

        private:
            void UpdateViewMatrix ();
            void UpdateProjectionMatrix ();

            Math::Vector3 m_Position = { 0.0f, 0.0f, 5.0f };
            Math::Vector3 m_Target = { 0.0f, 0.0f, 0.0f };
            Math::Vector3 m_Up = { 0.0f, 1.0f, 0.0f };

            float m_FOV = 60.0f;
            float m_AspectRatio = 16.0f / 9.0f;
            float m_NearPlane = 0.1f;
            float m_FarPlane = 1000.0f;

            Math::Matrix4 m_ViewMatrix;
            Math::Matrix4 m_ProjectionMatrix;

            bool m_ViewDirty = true;
            bool m_ProjectionDirty = true;
            bool m_IsOrthographic = false;
        };
    }