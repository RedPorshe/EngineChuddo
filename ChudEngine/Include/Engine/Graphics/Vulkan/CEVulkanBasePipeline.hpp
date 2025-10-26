// Runtime/Renderer/Vulkan/CEVulkanBasePipeline.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>
#include "CEVulkanBuffer.hpp"
#include "CEVulkanContext.hpp"
#include "Math/Matrix.hpp"

namespace CE
    {

    struct MatrixPushConstants
        {
            // Для column-major матриц НЕ нужно транспонировать!
        Math::Matrix4 modelMatrix;        // 64 байта
        Math::Matrix4 viewProjectionMatrix; // 64 байта

        MatrixPushConstants ()
            : modelMatrix ( Math::Matrix4::Identity () )
            , viewProjectionMatrix ( Math::Matrix4::Identity () )
            {
            }
        };

        // Проверка размера
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
        VkBlendFactor SrcBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor DstBlendFactor = VK_BLEND_FACTOR_ZERO;
        };

    class CEVulkanBasePipeline
        {
        public:
            CEVulkanBasePipeline ( CEVulkanContext * context, const PipelineConfig & config );
            virtual ~CEVulkanBasePipeline ();

            virtual bool Initialize ( VkRenderPass renderPass );
            virtual void Shutdown ();

            // Новые методы для совместимости
            void SetViewMatrix ( const Math::Matrix4 & view ) {
                // Сохраняем для использования в UpdateGlobalUniforms
                m_ViewMatrix = view;
                }

            void SetProjectionMatrix ( const Math::Matrix4 & projection ) {
                m_ProjectionMatrix = projection;
                }

            VkDescriptorSet GetDescriptorSet ( uint32_t frameIndex ) const {
                // Временная реализация - возвращаем null
                // TODO: реализовать систему descriptor sets
                return VK_NULL_HANDLE;
                }

            virtual void UpdatePerObjectUniforms ( uint32_t currentImage, uint32_t objectIndex,
                                                   const Math::Matrix4 & model, const Math::Vector4 & color )
                {
                    // Базовая реализация - можно оставить пустой
                CE_CORE_DEBUG ( "UpdatePerObjectUniforms called for pipeline '{}'", m_Config.Name );
                }

            virtual void BindVertexBuffer ( VkCommandBuffer commandBuffer )
                {
                    // Временная реализация
                CE_CORE_DEBUG ( "BindVertexBuffer called for pipeline '{}'", m_Config.Name );
                }

                // Common pipeline operations
            VkPipeline GetPipeline () const { return GraphicsPipeline; }
            VkPipelineLayout GetLayout () const { return this->PipelineLayout; }
            VkDescriptorSetLayout GetDescriptorSetLayout () const { return m_DescriptorSetLayout; }

            virtual void Bind ( VkCommandBuffer commandBuffer );
            virtual void UpdateGlobalUniforms ( uint32_t currentImage, const Math::Matrix4 & view, const Math::Matrix4 & projection );

            // Shader management
            bool SetVertexShader ( const std::string & shaderPath );
            bool SetFragmentShader ( const std::string & shaderPath );
            bool CompileShaders (); // ДОБАВЛЕНО
            bool ReloadShaders ();

            const std::string & GetName () const { return m_Config.Name; }

        protected:
            virtual bool CreateDescriptorSetLayout ();
            virtual bool CreatePipelineLayout ();
            virtual bool CreateGraphicsPipeline ( VkRenderPass renderPass );

            // Shader compilation (common for all pipelines)
            std::vector<uint32_t> CompileAndLoadShader ( const std::string & sourcePath ); // ДОБАВЛЕНО
            std::vector<uint32_t> LoadShader ( const std::string & filename ); // ДОБАВЛЕНО
            VkShaderModule CreateShaderModule ( const std::vector<uint32_t> & code );
            void DestroyShaderModules ();

            // Common resources
            CEVulkanContext * m_Context;
            PipelineConfig m_Config;

            VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
            VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
            VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;

            // Shader management
            std::string m_VertexShaderPath;
            std::string m_FragmentShaderPath;
            std::vector<uint32_t> m_VertexShaderCode;
            std::vector<uint32_t> m_FragmentShaderCode;
            VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
            VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;

            Math::Matrix4 m_ViewMatrix;
            Math::Matrix4 m_ProjectionMatrix;

            // Common uniform buffers
            struct GlobalUniforms
                {
                alignas( 16 ) Math::Matrix4 view;
                alignas( 16 ) Math::Matrix4 projection;
                alignas( 16 ) Math::Vector3 cameraPosition;
                };

            std::vector<std::unique_ptr<CEVulkanBuffer>> m_GlobalUniformBuffers;
            std::vector<void *> m_GlobalUniformBuffersMapped;
        };
    }