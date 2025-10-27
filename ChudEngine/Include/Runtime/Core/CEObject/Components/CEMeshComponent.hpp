#pragma once
#include "Core/CEObject/Components/CEComponent.hpp"
#include "Graphics/Vulkan/CEVulkanBuffer.hpp"
#include "Math/Vector.hpp"
#include <memory>
#include <array>
#include <vector>
#include <cstring> 

namespace CE
    {

    class CEVulkanRenderer; 
    class CEVulkanContext;  

    class CEMeshComponent : public CEComponent
        {
        public:
            CEMeshComponent ();
            virtual ~CEMeshComponent ();

            // Mesh data
            struct Vertex
                {
                Math::Vector3 Position;
                Math::Vector3 Color;

                static VkVertexInputBindingDescription GetBindingDescription () {
                    VkVertexInputBindingDescription bindingDescription {};
                    bindingDescription.binding = 0;
                    bindingDescription.stride = sizeof ( Vertex );
                    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                    
                    return bindingDescription;
                    }

                static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions () {
                    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};

                    // Position attribute
                    attributeDescriptions[ 0 ].binding = 0;
                    attributeDescriptions[ 0 ].location = 0;
                    attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
                    attributeDescriptions[ 0 ].offset = offsetof ( Vertex, Position );

                    // Color attribute  
                    attributeDescriptions[ 1 ].binding = 0;
                    attributeDescriptions[ 1 ].location = 1;
                    attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
                    attributeDescriptions[ 1 ].offset = offsetof ( Vertex, Color );

                    return attributeDescriptions;
                    }
                };

                // Mesh management
            void SetVertices ( const std::vector<Vertex> & vertices );
            void SetIndices ( const std::vector<uint32_t> & indices );
            bool CreateVertexBuffer ( CEVulkanRenderer * renderer );
            bool CreateIndexBuffer ( CEVulkanRenderer * renderer );
            void BindVertexBuffer ( VkCommandBuffer commandBuffer );
            void BindIndexBuffer ( VkCommandBuffer commandBuffer );
            void Render ( VkCommandBuffer commandBuffer );
            Math::Matrix4 GetTransformMatrix () const;
            bool EnsureBuffersCreated ( CEVulkanRenderer * renderer );

            // Getters
            bool IsValid () const { return m_VertexBuffer && m_VertexBuffer->IsValid (); }
            size_t GetVertexCount () const { return m_Vertices.size (); }
            size_t GetIndexCount () const { return m_Indices.size (); }
            bool HasIndices () const { return !m_Indices.empty (); }

            std::vector<Vertex> GetVertices () const { return m_Vertices; }
            std::vector<uint32_t> GetIndices () const { return m_Indices; }
            CEVulkanBuffer * GetIndexBuffer () const { return m_IndexBuffer.get (); }

        private:
            std::vector<Vertex> m_Vertices;
            std::vector<uint32_t> m_Indices;
            std::unique_ptr<CEVulkanBuffer> m_VertexBuffer;
            std::unique_ptr<CEVulkanBuffer> m_IndexBuffer;
            CEVulkanRenderer * m_Renderer = nullptr;
        };
    }