#include "Graphics/Vulkan/Meshes/CEVulkanMesh.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Memory/CEVulkanBuffer.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanMesh::CEVulkanMesh ()
        : m_Context ( nullptr )
        , m_VertexCount ( 0 )
        , m_IndexCount ( 0 )
        , m_Initialized ( false )
        {
        }

    CEVulkanMesh::~CEVulkanMesh ()
        {
        Shutdown ();
        }

    bool CEVulkanMesh::Initialize ( CEVulkanContext * context,
                                    const std::vector<Vertex> & vertices,
                                    const std::vector<uint32_t> & indices )
        {
        if (m_Initialized)
            {
            CE_CORE_WARN ( "Mesh already initialized" );
            return true;
            }

        m_Context = context;
        m_VertexCount = static_cast< uint32_t >( vertices.size () );
        m_IndexCount = static_cast< uint32_t >( indices.size () );

        try
            {
            if (!CreateBuffers ( vertices, indices ))
                {
                throw std::runtime_error ( "Failed to create mesh buffers" );
                }

            m_Initialized = true;
            CE_CORE_DEBUG ( "Mesh initialized successfully: {} vertices, {} indices", m_VertexCount, m_IndexCount );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize mesh: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanMesh::Shutdown ()
        {
        m_VertexBuffer.reset ();
        m_IndexBuffer.reset ();
        m_Material.reset ();

        m_VertexCount = 0;
        m_IndexCount = 0;
        m_Initialized = false;
        m_Context = nullptr;

        CE_CORE_DEBUG ( "Mesh shut down" );
        }

    void CEVulkanMesh::Bind ( VkCommandBuffer commandBuffer )
        {
        if (!m_Initialized) return;

        // Bind vertex buffer
        VkBuffer vertexBuffers [] = { m_VertexBuffer->GetBuffer () };
        VkDeviceSize offsets [] = { 0 };
        vkCmdBindVertexBuffers ( commandBuffer, 0, 1, vertexBuffers, offsets );

        // Bind index buffer
        vkCmdBindIndexBuffer ( commandBuffer, m_IndexBuffer->GetBuffer (), 0, VK_INDEX_TYPE_UINT32 );
        }

    void CEVulkanMesh::Draw ( VkCommandBuffer commandBuffer )
        {
        if (!m_Initialized || m_IndexCount == 0) return;

        vkCmdDrawIndexed ( commandBuffer, m_IndexCount, 1, 0, 0, 0 );
        }

    bool CEVulkanMesh::CreateBuffers ( const std::vector<Vertex> & vertices, const std::vector<uint32_t> & indices )
        {
        if (!m_Context)
            {
            CE_CORE_ERROR ( "No Vulkan context for buffer creation" );
            return false;
            }

            // Create vertex buffer
        VkDeviceSize vertexBufferSize = sizeof ( vertices[ 0 ] ) * vertices.size ();
        m_VertexBuffer = std::make_unique<CEVulkanBuffer> ();
        if (!m_VertexBuffer->Create ( m_Context, vertexBufferSize,
                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ))
            {
            CE_CORE_ERROR ( "Failed to create vertex buffer" );
            return false;
            }

            // Upload vertex data
        m_VertexBuffer->UploadData ( vertices.data (), vertexBufferSize );

        // Create index buffer
        VkDeviceSize indexBufferSize = sizeof ( indices[ 0 ] ) * indices.size ();
        m_IndexBuffer = std::make_unique<CEVulkanBuffer> ();
        if (!m_IndexBuffer->Create ( m_Context, indexBufferSize,
                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ))
            {
            CE_CORE_ERROR ( "Failed to create index buffer" );
            return false;
            }

            // Upload index data
        m_IndexBuffer->UploadData ( indices.data (), indexBufferSize );

        return true;
        }
    }