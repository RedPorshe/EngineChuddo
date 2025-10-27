// Graphics/Vulkan/Meshes/CEVulkanMesh.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Math/Vector.hpp"

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanBuffer;

    struct Vertex
        {
        Math::Vector3 position;
        Math::Vector3 normal;
        Math::Vector2 texCoord;
        Math::Vector3 tangent;
        Math::Vector3 bitangent;

        static VkVertexInputBindingDescription GetBindingDescription ();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions ();
        };

    class CEVulkanMesh
        {
        public:
            CEVulkanMesh ();
            ~CEVulkanMesh ();

            bool Initialize ( CEVulkanContext * context,
                              const std::vector<Vertex> & vertices,
                              const std::vector<uint32_t> & indices );
            void Shutdown ();

            void Bind ( VkCommandBuffer commandBuffer );
            void Draw ( VkCommandBuffer commandBuffer );

            void SetMaterial ( std::shared_ptr<class CEVulkanMaterial> material ) { m_Material = material; }
            std::shared_ptr<class CEVulkanMaterial> GetMaterial () const { return m_Material; }

            uint32_t GetVertexCount () const { return m_VertexCount; }
            uint32_t GetIndexCount () const { return m_IndexCount; }
            bool IsValid () const { return m_Initialized; }

        private:
            bool CreateBuffers ( const std::vector<Vertex> & vertices, const std::vector<uint32_t> & indices );

            CEVulkanContext * m_Context = nullptr;
            std::unique_ptr<CEVulkanBuffer> m_VertexBuffer;
            std::unique_ptr<CEVulkanBuffer> m_IndexBuffer;
            std::shared_ptr<class CEVulkanMaterial> m_Material;

            uint32_t m_VertexCount = 0;
            uint32_t m_IndexCount = 0;
            bool m_Initialized = false;
        };
    }