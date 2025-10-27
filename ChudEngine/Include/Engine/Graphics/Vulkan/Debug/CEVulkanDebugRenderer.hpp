// Graphics/Vulkan/Debug/CEVulkanDebugRenderer.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include <vector>
#include <memory>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanPipelineManager;
    class CEVulkanResourceManager;
    class CEVulkanBuffer;

    struct DebugVertex
        {
        Math::Vector3 position;
        Math::Vector3 color;
        };

    struct DebugLine
        {
        DebugVertex start;
        DebugVertex end;
        };

    class CEVulkanDebugRenderer
        {
        public:
            CEVulkanDebugRenderer ();
            ~CEVulkanDebugRenderer ();

            bool Initialize ( CEVulkanContext * context,
                              CEVulkanPipelineManager * pipelineManager,
                              CEVulkanResourceManager * resourceManager );
            void Shutdown ();

            void AddLine ( const Math::Vector3 & start, const Math::Vector3 & end, const Math::Vector3 & color );
            void AddBoundingBox ( const Math::Vector3 & min, const Math::Vector3 & max, const Math::Vector3 & color );
            void AddSphere ( const Math::Vector3 & center, float radius, const Math::Vector3 & color, uint32_t segments = 16 );
            void AddFrustum ( const Math::Matrix4 & viewProjection, const Math::Vector3 & color );
            void AddCoordinateSystem ( const Math::Vector3 & position, float scale = 1.0f );

            void Render ( VkCommandBuffer commandBuffer, const Math::Matrix4 & viewProjectionMatrix );
            void Clear ();

        private:
            bool CreateBuffers ();
            bool CreatePipeline ();
            void UpdateVertexBuffer ();

            CEVulkanContext * m_Context = nullptr;
            CEVulkanPipelineManager * m_PipelineManager = nullptr;
            CEVulkanResourceManager * m_ResourceManager = nullptr;

            std::vector<DebugLine> m_Lines;
            std::unique_ptr<CEVulkanBuffer> m_VertexBuffer;
            VkPipeline m_DebugPipeline = VK_NULL_HANDLE;
            VkPipelineLayout m_DebugPipelineLayout = VK_NULL_HANDLE;

            bool m_Initialized = false;
            bool m_Dirty = false;
        };
    }