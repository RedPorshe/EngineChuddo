#include "CEMeshComponent.hpp"
#include "Graphics/Vulkan/CEVulkanRenderer.hpp"
#include "Core/Logger.h"
#include <array>

namespace CE
    {

    Math::Matrix4 CEMeshComponent::GetTransformMatrix () const
        {
        if (GetOwner ())
            {
            auto * transform = GetOwner ()->GetTransform ();
            if (transform)
                {
                Math::Matrix4 worldTransform = transform->GetWorldTransform ();
                CE_DEBUG ( "Mesh '{}' world transform: position=({}, {}, {})",
                           GetName (),
                           transform->GetPosition ().x,
                           transform->GetPosition ().y,
                           transform->GetPosition ().z );
                return worldTransform;
                }
            }
        return Math::Matrix4::Identity ();
        }

    CEMeshComponent::CEMeshComponent ()
        {
        SetName ( "CEMeshComponent" );

   // Set default vertices
        m_Vertices = {
            {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
            };

        CE_DEBUG ( "CEMeshComponent '{}' created with {} vertices", GetName (), m_Vertices.size () );
        }

    bool CEMeshComponent::EnsureBuffersCreated ( CEVulkanRenderer * renderer )
        {
        if (!renderer) return false;

        // Создаем vertex buffer если нужно
        if (!m_VertexBuffer || !m_VertexBuffer->IsValid ())
            {
            if (!CreateVertexBuffer ( renderer ))
                {
                CE_ERROR ( "Failed to create vertex buffer for mesh '{}'", GetName () );
                return false;
                }
            }

            // Создаем index buffer если есть индексы
        if (HasIndices () && ( !m_IndexBuffer || !m_IndexBuffer->IsValid () ))
            {
            if (!CreateIndexBuffer ( renderer ))
                {
                CE_ERROR ( "Failed to create index buffer for mesh '{}'", GetName () );
                // Продолжаем без индексного буфера
                }
            }

        return IsValid ();
        }
   

    CEMeshComponent::~CEMeshComponent ()
        {
        if (m_VertexBuffer)
            {
            m_VertexBuffer->Destroy ();
            }
        if (m_IndexBuffer)
            {
            m_IndexBuffer->Destroy ();
            }
        CE_DEBUG ( "CEMeshComponent '{}' destroyed", GetName () );
        }

    void CEMeshComponent::SetVertices ( const std::vector<Vertex> & vertices )
        {
        m_Vertices = vertices;
        CE_DEBUG ( "CEMeshComponent '{}' set {} vertices", GetName (), vertices.size () );
        }

    void CEMeshComponent::SetIndices ( const std::vector<uint32_t> & indices )
        {
        m_Indices = indices;
        CE_DEBUG ( "CEMeshComponent '{}' set {} indices", GetName (), indices.size () );
        }

    bool CEMeshComponent::CreateVertexBuffer ( CEVulkanRenderer * renderer )
        {
        if (!renderer || m_Vertices.empty ())
            {
            return false;
            }

        CE_DEBUG ( "=== CEMeshComponent::CreateVertexBuffer ===" );
        CE_DEBUG ( "  Component: {}", GetName () );
        CE_DEBUG ( "  Vertex count: {}", m_Vertices.size () );
        CE_DEBUG ( "  Renderer valid: {}", ( bool ) renderer );

        if (m_Vertices.empty ())
            {
            CE_ERROR ( "  No vertices to create buffer!" );
            return false;
            }

        if (!renderer)
            {
            CE_ERROR ( "  Renderer is null!" );
            return false;
            }

        m_Renderer = renderer;

        if (m_Vertices.empty ())
            {
                // Default triangle if no vertices set
            m_Vertices = {
                {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
                };
            }

        VkDeviceSize bufferSize = sizeof ( Vertex ) * m_Vertices.size ();

        m_VertexBuffer = std::make_unique<CEVulkanBuffer> ();

        // Get context from renderer
        auto context = renderer->GetContext ();
        if (!context)
            {
            CE_ERROR ( "Cannot get Vulkan context from renderer" );
            return false;
            }

        bool success = m_VertexBuffer->Create (
            context,
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        if (success)
            {
            m_VertexBuffer->UploadData ( m_Vertices.data (), bufferSize );
            CE_DEBUG ( "CEMeshComponent '{}' vertex buffer created with {} vertices",
                       GetName (), m_Vertices.size () );
            }
        else
            {
            CE_ERROR ( "CEMeshComponent '{}' failed to create vertex buffer", GetName () );
            return false;
            }
        return true;
        }

    bool CEMeshComponent::CreateIndexBuffer ( CEVulkanRenderer * renderer )
        {
        if (!renderer || m_Indices.empty ())
            {
            return false;
            }

        m_Renderer = renderer;

        VkDeviceSize bufferSize = sizeof ( m_Indices[ 0 ] ) * m_Indices.size ();

        m_IndexBuffer = std::make_unique<CEVulkanBuffer> ();

        // Get context from renderer
        auto context = renderer->GetContext ();
        if (!context)
            {
            CE_ERROR ( "Cannot get Vulkan context from renderer" );
            return false;
            }

        bool success = m_IndexBuffer->Create (
            context,
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        if (success)
            {
            m_IndexBuffer->UploadData ( m_Indices.data (), bufferSize );
            CE_DEBUG ( "CEMeshComponent '{}' index buffer created with {} indices",
                       GetName (), m_Indices.size () );
            }
        else
            {
            CE_ERROR ( "CEMeshComponent '{}' failed to create index buffer", GetName () );
            return false;
            }
        return true;
        }

    void CEMeshComponent::BindVertexBuffer ( VkCommandBuffer commandBuffer )
        {
        if (!m_VertexBuffer || !m_VertexBuffer->IsValid ())
            return;

        VkBuffer vertexBuffers [] = { m_VertexBuffer->GetBuffer () };
        VkDeviceSize offsets [] = { 0 };
        vkCmdBindVertexBuffers ( commandBuffer, 0, 1, vertexBuffers, offsets );
        CE_INFO ( "BindVertexBuffer ( VkCommandBuffer commandBuffer ) complited" );
        }

    void CEMeshComponent::BindIndexBuffer ( VkCommandBuffer commandBuffer )
        {
        if (!m_IndexBuffer || !m_IndexBuffer->IsValid ())
            return;

        vkCmdBindIndexBuffer ( commandBuffer, m_IndexBuffer->GetBuffer (), 0, VK_INDEX_TYPE_UINT32 );
        CE_INFO ( "BindIndexBuffer ( VkCommandBuffer commandBuffer ) complited" );
        }

    void CEMeshComponent::Render ( VkCommandBuffer commandBuffer )
        {
        if (!EnsureBuffersCreated ( m_Renderer ))
            {
            CE_ERROR ( "Failed to ensure buffers for mesh component '{}'", GetName () );
            return;
            }

        if (!m_VertexBuffer || !m_VertexBuffer->IsValid ())
            {
            CE_ERROR ( "Vertex buffer not valid for mesh component '{}'", GetName () );
            return;
            }

            // Биндим vertex buffer
        BindVertexBuffer ( commandBuffer );
        CE_INFO ( "Vertex buffer binded" );
        // Рендерим с индексами или без
        if (HasIndices () && m_IndexBuffer && m_IndexBuffer->IsValid ())
            {
            BindIndexBuffer ( commandBuffer );
            vkCmdDrawIndexed ( commandBuffer, static_cast< uint32_t >( m_Indices.size () ), 1, 0, 0, 0 );
            CE_DEBUG ( "Rendered mesh '{}' with {} indices", GetName (), m_Indices.size () );
            }
        else
            {
            vkCmdDraw ( commandBuffer, static_cast< uint32_t >( m_Vertices.size () ), 1, 0, 0 );
            CE_DEBUG ( "Rendered mesh '{}' with {} vertices", GetName (), m_Vertices.size () );
            }
        }
    }