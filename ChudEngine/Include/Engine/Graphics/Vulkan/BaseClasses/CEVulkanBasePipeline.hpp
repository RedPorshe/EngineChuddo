// Graphics/Vulkan/BaseClasses/CEVulkanBasePipeline.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include "Math/Matrix.hpp"

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanShaderManager;

    struct MatrixPushConstants
        {
        Math::Matrix4 modelMatrix;
        Math::Matrix4 viewProjectionMatrix;

        MatrixPushConstants ()
            : modelMatrix ( Math::Matrix4::Identity () )
            , viewProjectionMatrix ( Math::Matrix4::Identity () )
            {
            }
        };

    static_assert( sizeof ( MatrixPushConstants ) == 128, "MatrixPushConstants size mismatch!" );

    struct PipelineConfig
        {
        std::string Name;
        VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        bool DepthTest = true;
        bool DepthWrite = true;
        bool BlendEnable = false;
        VkSampleCountFlagBits MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
        };

    class CEVulkanBasePipeline
        {
        public:
            CEVulkanBasePipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager, const PipelineConfig & config );
            virtual ~CEVulkanBasePipeline ();

            virtual bool Initialize ( VkRenderPass renderPass );
            virtual void Shutdown ();
            virtual void Bind ( VkCommandBuffer commandBuffer );

            bool SetVertexShader ( const std::string & shaderPath );
            bool SetFragmentShader ( const std::string & shaderPath );
            bool ReloadShaders ();

            VkPipeline GetPipeline () const { return m_GraphicsPipeline; }
            VkPipelineLayout GetLayout () const { return m_PipelineLayout; }
            VkDescriptorSetLayout GetDescriptorSetLayout () const { return m_DescriptorSetLayout; }
            const std::string & GetName () const { return m_Config.Name; }

        protected:
            virtual bool CreateDescriptorSetLayout ();
            virtual bool CreatePipelineLayout ();
            virtual bool CreateGraphicsPipeline ( VkRenderPass renderPass );

            VkPipelineShaderStageCreateInfo CreateShaderStageInfo (
                std::shared_ptr<CEVulkanShaderManager::ShaderModule> shader );

            CEVulkanContext * m_Context = nullptr;
            CEVulkanShaderManager * m_ShaderManager = nullptr;
            PipelineConfig m_Config;

            VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
            VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
            VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;

            std::shared_ptr<CEVulkanShaderManager::ShaderModule> m_VertexShader;
            std::shared_ptr<CEVulkanShaderManager::ShaderModule> m_FragmentShader;
            std::string m_VertexShaderPath;
            std::string m_FragmentShaderPath;
        };
    }