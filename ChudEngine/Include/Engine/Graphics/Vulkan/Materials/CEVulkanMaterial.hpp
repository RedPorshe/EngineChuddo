// Graphics/Vulkan/Materials/CEVulkanMaterial.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "Math/Vector.hpp"

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanPipelineManager;
    class CEVulkanTextureManager;

    struct MaterialParameter
        {
        enum class Type
            {
            Float,
            Vector2,
            Vector3,
            Vector4,
            Matrix4,
            Texture,
            Integer,
            Boolean
            };

        Type type;
        union
            {
            float floatValue;
            int intValue;
            bool boolValue;
            Math::Vector2 vector2Value;
            Math::Vector3 vector3Value;
            Math::Vector4 vector4Value;
            };
        std::shared_ptr<class CEVulkanTexture> textureValue;
        Math::Matrix4 matrixValue;
        };

    class CEVulkanMaterial
        {
        public:
            CEVulkanMaterial ();
            ~CEVulkanMaterial ();

            bool Initialize ( CEVulkanContext * context,
                              CEVulkanPipelineManager * pipelineManager,
                              CEVulkanTextureManager * textureManager,
                              const std::string & name );
            void Shutdown ();

            void SetParameter ( const std::string & name, const MaterialParameter & parameter );
            MaterialParameter GetParameter ( const std::string & name ) const;
            bool HasParameter ( const std::string & name ) const;

            void SetPipelineType ( PipelineType type ) { m_PipelineType = type; }
            PipelineType GetPipelineType () const { return m_PipelineType; }

            void Bind ( VkCommandBuffer commandBuffer );
            void UpdateUniforms ( uint32_t currentFrame );

            const std::string & GetName () const { return m_Name; }
            bool IsTransparent () const { return m_IsTransparent; }
            void SetTransparent ( bool transparent ) { m_IsTransparent = transparent; }

        private:
            bool CreateDescriptorSets ();
            void UpdateDescriptorSets ();

            CEVulkanContext * m_Context = nullptr;
            CEVulkanPipelineManager * m_PipelineManager = nullptr;
            CEVulkanTextureManager * m_TextureManager = nullptr;

            std::string m_Name;
            PipelineType m_PipelineType = PipelineType::StaticMesh;
            std::unordered_map<std::string, MaterialParameter> m_Parameters;

            std::vector<VkDescriptorSet> m_DescriptorSets;
            bool m_IsTransparent = false;
            bool m_Initialized = false;
        };
    }