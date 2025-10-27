// Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanShaderManager
        {
        public:
            struct ShaderModule
                {
                VkShaderModule module = VK_NULL_HANDLE;
                std::vector<uint32_t> code;
                std::string path;
                VkShaderStageFlagBits stage;
                std::string entryPoint = "main";
                };

            CEVulkanShaderManager ( CEVulkanContext * context );
            ~CEVulkanShaderManager ();

            std::shared_ptr<ShaderModule> LoadShader ( const std::string & filename, VkShaderStageFlagBits stage );
            std::shared_ptr<ShaderModule> CompileAndLoadShader ( const std::string & sourcePath, VkShaderStageFlagBits stage );
            void ReloadAllShaders ();

            void DestroyShaderModule ( std::shared_ptr<ShaderModule> shaderModule );
            void Cleanup ();

            static VkShaderStageFlagBits GetShaderStageFromExtension ( const std::string & filename );
            static std::string GetShaderStageName ( VkShaderStageFlagBits stage );

        private:
            std::vector<uint32_t> ReadSPIRVFile ( const std::string & filename );
            bool CompileGLSLToSPIRV ( const std::string & sourcePath, const std::string & outputPath );
            VkShaderModule CreateShaderModule ( const std::vector<uint32_t> & code );

            CEVulkanContext * m_Context = nullptr;
            std::unordered_map<std::string, std::shared_ptr<ShaderModule>> m_ShaderModules;
        };
    }