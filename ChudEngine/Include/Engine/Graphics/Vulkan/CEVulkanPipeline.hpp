// Runtime/Renderer/Vulkan/CEVulkanPipeline.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "CEVulkanBuffer.hpp"
#include "CEVulkanContext.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include <array>
#include <memory>
#include <string>

namespace CE
    {
    struct UniformBufferObject
        {
        alignas( 16 ) Math::Matrix4 model;
        alignas( 16 ) Math::Matrix4 view;
        alignas( 16 ) Math::Matrix4 proj;
        };

    class CEVulkanPipeline
        {
        public:
            CEVulkanPipeline ( CEVulkanContext * context );
            ~CEVulkanPipeline ();

            bool Initialize ( VkRenderPass renderPass );
            void Shutdown ();

            VkPipeline GetPipeline () const { return GraphicsPipeline; }
            VkPipelineLayout GetLayout () const { return PipelineLayout; }
            VkDescriptorSet GetDescriptorSet ( uint32_t frameIndex ) const {
                return ( frameIndex < m_DescriptorSets.size () ) ? m_DescriptorSets[ frameIndex ] : VK_NULL_HANDLE;
                }

            bool CreateVertexBuffer ();
            void BindVertexBuffer ( VkCommandBuffer commandBuffer );

            // Новые методы для uniform buffers
            void SetViewMatrix ( const Math::Matrix4 & viewMatrix ) { m_ViewMatrix = viewMatrix; }
            void SetProjectionMatrix ( const Math::Matrix4 & projectionMatrix ) { m_ProjectionMatrix = projectionMatrix; }
            void UpdateUniforms ( uint32_t currentImage, const Math::Matrix4 & modelMatrix );
            VkBuffer GetUniformBuffer ( uint32_t frameIndex ) const;

            void UpdateDynamicUniforms ( uint32_t currentImage, uint32_t objectIndex, const Math::Matrix4 & modelMatrix );
            VkDescriptorBufferInfo GetDynamicUniformBufferInfo ( uint32_t frameIndex, uint32_t objectIndex ) const;
            uint32_t GetMaxObjects () const { return MAX_OBJECTS; }
            VkDeviceSize GetDynamicAlignment () const { return m_DynamicAlignment; }

            // Новые методы для управления шейдерами
            bool SetVertexShader ( const std::string & shaderPath );
            bool SetFragmentShader ( const std::string & shaderPath );
            bool CompileShaders ();
            bool ReloadShaders ();

            // Геттеры для отладки
            const std::string & GetVertexShaderPath () const { return m_VertexShaderPath; }
            const std::string & GetFragmentShaderPath () const { return m_FragmentShaderPath; }

        private:
            bool CreateGraphicsPipeline ( VkRenderPass renderPass );
            bool CreateUniformBuffers ();
            bool CreateDescriptorSetLayout ();
            bool CreateDescriptorPool ();
            bool CreateDescriptorSets ();
            VkShaderModule CreateShaderModule ( const std::vector<uint32_t> & code );
            void DestroyShaderModules ();

            // Методы работы с шейдерами
            std::vector<uint32_t> LoadShader ( const std::string & filename );
            std::vector<uint32_t> CompileAndLoadShader ( const std::string & sourcePath );
            bool CompileShaderFile ( const std::string & sourcePath, const std::string & outputPath );

            // Устаревшие методы - сохраняем для совместимости
            std::vector<uint32_t> CompileVertexShader ();
            std::vector<uint32_t> CompileFragmentShader ();

            CEVulkanContext * m_Context;

            VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
            VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
            VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
            VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
            std::vector<VkDescriptorSet> m_DescriptorSets;

            std::unique_ptr<CEVulkanBuffer> m_VertexBuffer;
            std::vector<std::unique_ptr<CEVulkanBuffer>> m_UniformBuffers;
            std::vector<void *> m_UniformBuffersMapped;

            std::vector<std::unique_ptr<CEVulkanBuffer>> m_DynamicUniformBuffers;
            std::vector<void *> m_DynamicUniformBuffersMapped;

            // Выравнивание
            VkDeviceSize m_UniformBufferAlignment = 0;
            VkDeviceSize m_DynamicAlignment = 0;

            static const uint32_t MAX_OBJECTS = 1000;

            Math::Matrix4 m_ViewMatrix;
            Math::Matrix4 m_ProjectionMatrix;

            // Пути к шейдерам
            std::string m_VertexShaderPath;
            std::string m_FragmentShaderPath;
            std::vector<uint32_t> m_VertexShaderCode;
            std::vector<uint32_t> m_FragmentShaderCode;

            // Шейдерные модули (для cleanup)
            VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
            VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;

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

                    attributeDescriptions[ 0 ].binding = 0;
                    attributeDescriptions[ 0 ].location = 0;
                    attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
                    attributeDescriptions[ 0 ].offset = offsetof ( Vertex, Position );

                    attributeDescriptions[ 1 ].binding = 0;
                    attributeDescriptions[ 1 ].location = 1;
                    attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
                    attributeDescriptions[ 1 ].offset = offsetof ( Vertex, Color );

                    return attributeDescriptions;
                    }
                };
        };
    }