#include "Graphics/Vulkan/Debug/CEVulkanDebugRenderer.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanPipelineManager.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanResourceManager.hpp"
#include "Graphics/Vulkan/Memory/CEVulkanBuffer.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanDebugRenderer::CEVulkanDebugRenderer ()
        : m_Context ( nullptr )
        , m_PipelineManager ( nullptr )
        , m_ResourceManager ( nullptr )
        , m_DebugPipeline ( VK_NULL_HANDLE )
        , m_DebugPipelineLayout ( VK_NULL_HANDLE )
        , m_Initialized ( false )
        , m_Dirty ( false )
        {
        }

    CEVulkanDebugRenderer::~CEVulkanDebugRenderer ()
        {
        Shutdown ();
        }

    bool CEVulkanDebugRenderer::Initialize ( CEVulkanContext * context,
                                             CEVulkanPipelineManager * pipelineManager,
                                             CEVulkanResourceManager * resourceManager )
        {
        if (m_Initialized)
            {
            return true;
            }

        m_Context = context;
        m_PipelineManager = pipelineManager;
        m_ResourceManager = resourceManager;

        try
            {
            if (!CreateBuffers ())
                {
                throw std::runtime_error ( "Failed to create debug buffers" );
                }

            if (!CreatePipeline ())
                {
                throw std::runtime_error ( "Failed to create debug pipeline" );
                }

            m_Initialized = true;
            CE_CORE_DEBUG ( "Debug renderer initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize debug renderer: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanDebugRenderer::Shutdown ()
        {
        m_VertexBuffer.reset ();

        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            if (m_DebugPipeline != VK_NULL_HANDLE)
                {
                vkDestroyPipeline ( device, m_DebugPipeline, nullptr );
                m_DebugPipeline = VK_NULL_HANDLE;
                }

            if (m_DebugPipelineLayout != VK_NULL_HANDLE)
                {
                vkDestroyPipelineLayout ( device, m_DebugPipelineLayout, nullptr );
                m_DebugPipelineLayout = VK_NULL_HANDLE;
                }
            }

        m_Lines.clear ();
        m_Initialized = false;
        m_Dirty = false;

        CE_CORE_DEBUG ( "Debug renderer shut down" );
        }

    void CEVulkanDebugRenderer::AddLine ( const Math::Vector3 & start, const Math::Vector3 & end, const Math::Vector3 & color )
        {
        DebugLine line;
        line.start.position = start;
        line.start.color = color;
        line.end.position = end;
        line.end.color = color;

        m_Lines.push_back ( line );
        m_Dirty = true;
        }

    void CEVulkanDebugRenderer::AddBoundingBox ( const Math::Vector3 & min, const Math::Vector3 & max, const Math::Vector3 & color )
        {
            // Create 12 lines for a bounding box
        Math::Vector3 vertices[ 8 ] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {max.x, max.y, min.z},
            {min.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {max.x, max.y, max.z},
            {min.x, max.y, max.z}
            };

            // Bottom face
        AddLine ( vertices[ 0 ], vertices[ 1 ], color );
        AddLine ( vertices[ 1 ], vertices[ 2 ], color );
        AddLine ( vertices[ 2 ], vertices[ 3 ], color );
        AddLine ( vertices[ 3 ], vertices[ 0 ], color );

        // Top face
        AddLine ( vertices[ 4 ], vertices[ 5 ], color );
        AddLine ( vertices[ 5 ], vertices[ 6 ], color );
        AddLine ( vertices[ 6 ], vertices[ 7 ], color );
        AddLine ( vertices[ 7 ], vertices[ 4 ], color );

        // Vertical edges
        AddLine ( vertices[ 0 ], vertices[ 4 ], color );
        AddLine ( vertices[ 1 ], vertices[ 5 ], color );
        AddLine ( vertices[ 2 ], vertices[ 6 ], color );
        AddLine ( vertices[ 3 ], vertices[ 7 ], color );
        }

    void CEVulkanDebugRenderer::AddSphere ( const Math::Vector3 & center, float radius, const Math::Vector3 & color, uint32_t segments )
        {
            // Create a wireframe sphere using multiple circles
        const float pi = 3.14159265359f;

        // Horizontal circles
        for (uint32_t i = 0; i <= segments; ++i)
            {
            float theta1 = 2.0f * pi * i / segments;
            float theta2 = 2.0f * pi * ( i + 1 ) / segments;

            for (uint32_t j = 0; j <= segments; ++j)
                {
                float phi1 = pi * j / segments;
                float phi2 = pi * ( j + 1 ) / segments;

                Math::Vector3 p1 = center + Math::Vector3 (
                    radius * sin ( phi1 ) * cos ( theta1 ),
                    radius * cos ( phi1 ),
                    radius * sin ( phi1 ) * sin ( theta1 )
                );

                Math::Vector3 p2 = center + Math::Vector3 (
                    radius * sin ( phi1 ) * cos ( theta2 ),
                    radius * cos ( phi1 ),
                    radius * sin ( phi1 ) * sin ( theta2 )
                );

                Math::Vector3 p3 = center + Math::Vector3 (
                    radius * sin ( phi2 ) * cos ( theta1 ),
                    radius * cos ( phi2 ),
                    radius * sin ( phi2 ) * sin ( theta1 )
                );

                AddLine ( p1, p2, color );
                AddLine ( p1, p3, color );
                }
            }
        }

    void CEVulkanDebugRenderer::AddFrustum ( const Math::Matrix4 & viewProjection, const Math::Vector3 & color )
        {
            // Extract frustum corners from view-projection matrix
            // This is a simplified implementation
        Math::Vector3 corners[ 8 ];

        // Near plane corners
        corners[ 0 ] = Math::Vector3 ( -1, -1, 0 ); // bottom-left near
        corners[ 1 ] = Math::Vector3 ( 1, -1, 0 );  // bottom-right near
        corners[ 2 ] = Math::Vector3 ( 1, 1, 0 );   // top-right near
        corners[ 3 ] = Math::Vector3 ( -1, 1, 0 );  // top-left near

        // Far plane corners
        corners[ 4 ] = Math::Vector3 ( -1, -1, 1 ); // bottom-left far
        corners[ 5 ] = Math::Vector3 ( 1, -1, 1 );  // bottom-right far
        corners[ 6 ] = Math::Vector3 ( 1, 1, 1 );   // top-right far
        corners[ 7 ] = Math::Vector3 ( -1, 1, 1 );  // top-left far

        // Transform corners to world space (simplified)
        // In a real implementation, you'd unproject these points

        // Draw near plane
        AddLine ( corners[ 0 ], corners[ 1 ], color );
        AddLine ( corners[ 1 ], corners[ 2 ], color );
        AddLine ( corners[ 2 ], corners[ 3 ], color );
        AddLine ( corners[ 3 ], corners[ 0 ], color );

        // Draw far plane
        AddLine ( corners[ 4 ], corners[ 5 ], color );
        AddLine ( corners[ 5 ], corners[ 6 ], color );
        AddLine ( corners[ 6 ], corners[ 7 ], color );
        AddLine ( corners[ 7 ], corners[ 4 ], color );

        // Connect near and far planes
        AddLine ( corners[ 0 ], corners[ 4 ], color );
        AddLine ( corners[ 1 ], corners[ 5 ], color );
        AddLine ( corners[ 2 ], corners[ 6 ], color );
        AddLine ( corners[ 3 ], corners[ 7 ], color );
        }

    void CEVulkanDebugRenderer::AddCoordinateSystem ( const Math::Vector3 & position, float scale )
        {
            // X axis (red)
        AddLine ( position, position + Math::Vector3 ( scale, 0, 0 ), Math::Vector3 ( 1, 0, 0 ) );
        // Y axis (green)
        AddLine ( position, position + Math::Vector3 ( 0, scale, 0 ), Math::Vector3 ( 0, 1, 0 ) );
        // Z axis (blue)
        AddLine ( position, position + Math::Vector3 ( 0, 0, scale ), Math::Vector3 ( 0, 0, 1 ) );
        }

    void CEVulkanDebugRenderer::Render ( VkCommandBuffer commandBuffer, const Math::Matrix4 & viewProjectionMatrix )
        {
        if (!m_Initialized || m_Lines.empty ())
            {
            return;
            }

            // Update vertex buffer if needed
        if (m_Dirty)
            {
            UpdateVertexBuffer ();
            m_Dirty = false;
            }

            // Bind debug pipeline
        vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugPipeline );

        // Set view-projection matrix as push constant
        vkCmdPushConstants (
            commandBuffer,
            m_DebugPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof ( Math::Matrix4 ),
            &viewProjectionMatrix
        );

        // Bind vertex buffer
        VkBuffer vertexBuffers [] = { m_VertexBuffer->GetBuffer () };
        VkDeviceSize offsets [] = { 0 };
        vkCmdBindVertexBuffers ( commandBuffer, 0, 1, vertexBuffers, offsets );

        // Draw lines
        vkCmdDraw ( commandBuffer, static_cast< uint32_t >( m_Lines.size () * 2 ), 1, 0, 0 );
        }

    void CEVulkanDebugRenderer::Clear ()
        {
        m_Lines.clear ();
        m_Dirty = true;
        }

    bool CEVulkanDebugRenderer::CreateBuffers ()
        {
        if (!m_ResourceManager)
            {
            return false;
            }

            // Create a reasonably sized vertex buffer for debug lines
            // We'll resize it dynamically as needed
        const VkDeviceSize initialSize = sizeof ( DebugVertex ) * 1000; // Space for 500 lines

        m_VertexBuffer = m_ResourceManager->CreateBuffer (
            "DebugVertexBuffer",
            initialSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        return m_VertexBuffer != nullptr;
        }

    bool CEVulkanDebugRenderer::CreatePipeline ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            return false;
            }

            // This would create a simple line rendering pipeline
            // For now, we'll use the debug pipeline from pipeline manager

        auto debugPipeline = m_PipelineManager->GetPipeline ( PipelineType::Debug );
        if (debugPipeline)
            {
            m_DebugPipeline = debugPipeline->GetPipeline ();
            m_DebugPipelineLayout = debugPipeline->GetLayout ();
            return true;
            }

        return false;
        }

    void CEVulkanDebugRenderer::UpdateVertexBuffer ()
        {
        if (!m_VertexBuffer || m_Lines.empty ())
            {
            return;
            }

            // Convert lines to vertex data
        std::vector<DebugVertex> vertices;
        vertices.reserve ( m_Lines.size () * 2 );

        for (const auto & line : m_Lines)
            {
            vertices.push_back ( line.start );
            vertices.push_back ( line.end );
            }

            // Check if we need to resize the buffer
        VkDeviceSize requiredSize = vertices.size () * sizeof ( DebugVertex );
        if (requiredSize > m_VertexBuffer->GetSize ())
            {
// Resize buffer (in a real implementation, you'd create a new buffer)
            CE_CORE_WARN ( "Debug vertex buffer too small, need to implement resizing" );
            return;
            }

            // Upload vertex data
        m_VertexBuffer->UploadData ( vertices.data (), requiredSize );
        }
    }