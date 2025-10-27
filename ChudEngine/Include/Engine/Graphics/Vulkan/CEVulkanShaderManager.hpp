// Runtime/Renderer/Vulkan/CEVulkanShaderManager.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <memory>
#include "Core/Logger.h"

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanShaderManager
        {
        public:
            CEVulkanShaderManager ( CEVulkanContext * context );
            ~CEVulkanShaderManager ();

            struct ShaderModule
                {
                VkShaderModule module = VK_NULL_HANDLE;
                std::vector<uint32_t> code;
                std::string path;
                VkShaderStageFlagBits stage;
                };

                // Основные операции
            std::shared_ptr<ShaderModule> LoadShader ( const std::string & filename, VkShaderStageFlagBits stage );
            std::shared_ptr<ShaderModule> CompileAndLoadShader ( const std::string & sourcePath, VkShaderStageFlagBits stage );
            void ReloadAllShaders ();

            // Управление модулями
            void DestroyShaderModule ( std::shared_ptr<ShaderModule> shaderModule );
            void Cleanup ();

            // Вспомогательные методы
            static VkShaderStageFlagBits GetShaderStageFromExtension ( const std::string & filename );

        private:
            std::vector<uint32_t> ReadSPIRVFile ( const std::string & filename );
            bool CompileGLSLToSPIRV ( const std::string & sourcePath, const std::string & outputPath );
            VkShaderModule CreateShaderModule ( const std::vector<uint32_t> & code );

            CEVulkanContext * m_Context = nullptr;
            std::vector<std::shared_ptr<ShaderModule>> m_ShaderModules;
        };
    }