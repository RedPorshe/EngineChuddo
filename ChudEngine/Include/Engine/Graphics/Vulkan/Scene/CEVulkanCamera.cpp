#include "Graphics/Vulkan/Scene/CEVulkanCamera.hpp"
#include "Utils/Logger.hpp"
#include <cmath>

namespace CE
    {
    CEVulkanCamera::CEVulkanCamera ()
        : m_Position ( 0.0f, 0.0f, 5.0f )
        , m_Target ( 0.0f, 0.0f, 0.0f )
        , m_Up ( 0.0f, 1.0f, 0.0f )
        , m_FOV ( 60.0f )
        , m_AspectRatio ( 16.0f / 9.0f )
        , m_NearPlane ( 0.1f )
        , m_FarPlane ( 1000.0f )
        , m_ViewDirty ( true )
        , m_ProjectionDirty ( true )
        , m_IsOrthographic ( false )
        {
        m_ViewMatrix = Math::Matrix4::Identity ();
        m_ProjectionMatrix = Math::Matrix4::Identity ();
        }

    void CEVulkanCamera::LookAt ( const Math::Vector3 & position, const Math::Vector3 & target, const Math::Vector3 & up )
        {
        m_Position = position;
        m_Target = target;
        m_Up = up;
        m_ViewDirty = true;
        }

    void CEVulkanCamera::SetPerspective ( float fov, float aspectRatio, float nearPlane, float farPlane )
        {
        m_FOV = fov;
        m_AspectRatio = aspectRatio;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
        m_IsOrthographic = false;
        m_ProjectionDirty = true;
        }

    void CEVulkanCamera::SetOrthographic ( float left, float right, float bottom, float top, float nearPlane, float farPlane )
        {
            // For orthographic projection, we'd store these parameters
            // For now, we'll just mark that we're using orthographic
        m_IsOrthographic = true;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
        m_ProjectionDirty = true;

        // TODO: Store orthographic parameters and implement orthographic projection
        CE_CORE_WARN ( "Orthographic projection not fully implemented" );
        }

    const Math::Matrix4 & CEVulkanCamera::GetViewMatrix ()
        {
        if (m_ViewDirty)
            {
            UpdateViewMatrix ();
            }
        return m_ViewMatrix;
        }

    const Math::Matrix4 & CEVulkanCamera::GetProjectionMatrix ()
        {
        if (m_ProjectionDirty)
            {
            UpdateProjectionMatrix ();
            }
        return m_ProjectionMatrix;
        }

    Math::Matrix4 CEVulkanCamera::GetViewProjectionMatrix ()
        {
        return GetProjectionMatrix () * GetViewMatrix ();
        }

    Math::Vector3 CEVulkanCamera::GetForward () const
        {
        return Math::Normalize ( m_Target - m_Position );
        }

    Math::Vector3 CEVulkanCamera::GetRight () const
        {
        return Math::Normalize ( Math::Cross ( GetForward (), m_Up ) );
        }

    Math::Vector3 CEVulkanCamera::GetUp () const
        {
        return Math::Normalize ( Math::Cross ( GetRight (), GetForward () ) );
        }

    void CEVulkanCamera::UpdateViewMatrix ()
        {
        m_ViewMatrix = Math::LookAt ( m_Position, m_Target, m_Up );
        m_ViewDirty = false;
        }

    void CEVulkanCamera::UpdateProjectionMatrix ()
        {
        if (m_IsOrthographic)
            {
// TODO: Implement orthographic projection
            m_ProjectionMatrix = Math::Matrix4::Identity ();
            }
        else
            {
                 // Vulkan uses a different coordinate system (Y down, Z [0,1])
                 // We need to adjust the projection matrix for Vulkan
            m_ProjectionMatrix = Math::Perspective (
                m_FOV * 3.14159f / 180.0f, // Convert to radians
                m_AspectRatio,
                m_NearPlane,
                m_FarPlane
            );

            // Flip Y axis for Vulkan
            m_ProjectionMatrix[ 1 ][ 1 ] *= -1;
            }
        m_ProjectionDirty = false;
        }
    }