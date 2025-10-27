#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace CE
    {
    CEVulkanShaderManager::CEVulkanShaderManager ( CEVulkanContext * context )
        : m_Context ( context )
        {
        }

    CEVulkanShaderManager::~CEVulkanShaderManager ()
        {
        Cleanup ();
        }

    std::shared_ptr<CEVulkanShaderManager::ShaderModule> CEVulkanShaderManager::LoadShader (
        const std::string & filename, VkShaderStageFlagBits stage )
        {
            // Check if shader is already loaded
        auto it = m_ShaderModules.find ( filename );
        if (it != m_ShaderModules.end ())
            {
            return it->second;
            }

        try
            {
            std::vector<uint32_t> shaderCode = ReadSPIRVFile ( filename );
            VkShaderModule shaderModule = CreateShaderModule ( shaderCode );

            auto shader = std::make_shared<ShaderModule> ();
            shader->module = shaderModule;
            shader->code = std::move ( shaderCode );
            shader->path = filename;
            shader->stage = stage;

            m_ShaderModules[ filename ] = shader;
            CE_CORE_DEBUG ( "Loaded shader: {} (stage: {})", filename, GetShaderStageName ( stage ) );

            return shader;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to load shader {}: {}", filename, e.what () );
                return nullptr;
                }
        }

    std::shared_ptr<CEVulkanShaderManager::ShaderModule> CEVulkanShaderManager::CompileAndLoadShader (
        const std::string & sourcePath, VkShaderStageFlagBits stage )
        {
            // For now, we assume pre-compiled SPIR-V files
            // In a real implementation, this would invoke glslangValidator or similar
        std::string compiledPath = sourcePath + ".spv";
        return LoadShader ( compiledPath, stage );
        }

    void CEVulkanShaderManager::ReloadAllShaders ()
        {
        CE_CORE_INFO ( "Reloading all shaders..." );

        for (auto & [filename, shader] : m_ShaderModules)
            {
            try
                {
                std::vector<uint32_t> newCode = ReadSPIRVFile ( filename );
                VkShaderModule newModule = CreateShaderModule ( newCode );

                // Destroy old module
                if (shader->module != VK_NULL_HANDLE)
                    {
                    vkDestroyShaderModule ( m_Context->GetDevice ()->GetDevice (), shader->module, nullptr );
                    }

                    // Update with new module
                shader->module = newModule;
                shader->code = std::move ( newCode );

                CE_CORE_DEBUG ( "Reloaded shader: {}", filename );
                }
                catch (const std::exception & e)
                    {
                    CE_CORE_ERROR ( "Failed to reload shader {}: {}", filename, e.what () );
                    }
            }
        }

    void CEVulkanShaderManager::DestroyShaderModule ( std::shared_ptr<ShaderModule> shaderModule )
        {
        if (shaderModule && shaderModule->module != VK_NULL_HANDLE)
            {
            vkDestroyShaderModule ( m_Context->GetDevice ()->GetDevice (), shaderModule->module, nullptr );
            shaderModule->module = VK_NULL_HANDLE;
            }
        }

    void CEVulkanShaderManager::Cleanup ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            for (auto & [filename, shader] : m_ShaderModules)
                {
                if (shader->module != VK_NULL_HANDLE)
                    {
                    vkDestroyShaderModule ( device, shader->module, nullptr );
                    shader->module = VK_NULL_HANDLE;
                    }
                }
            }

        m_ShaderModules.clear ();
        CE_CORE_DEBUG ( "Shader manager cleaned up" );
        }

    VkShaderStageFlagBits CEVulkanShaderManager::GetShaderStageFromExtension ( const std::string & filename )
        {
        size_t dotPos = filename.find_last_of ( '.' );
        if (dotPos == std::string::npos)
            {
            return VK_SHADER_STAGE_ALL;
            }

        std::string extension = filename.substr ( dotPos + 1 );

        if (extension == "vert") return VK_SHADER_STAGE_VERTEX_BIT;
        if (extension == "frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
        if (extension == "comp") return VK_SHADER_STAGE_COMPUTE_BIT;
        if (extension == "geom") return VK_SHADER_STAGE_GEOMETRY_BIT;
        if (extension == "tesc") return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (extension == "tese") return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

        return VK_SHADER_STAGE_ALL;
        }

    std::string CEVulkanShaderManager::GetShaderStageName ( VkShaderStageFlagBits stage )
        {
        switch (stage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT: return "Vertex";
                case VK_SHADER_STAGE_FRAGMENT_BIT: return "Fragment";
                case VK_SHADER_STAGE_COMPUTE_BIT: return "Compute";
                case VK_SHADER_STAGE_GEOMETRY_BIT: return "Geometry";
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "Tessellation Control";
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "Tessellation Evaluation";
                default: return "Unknown";
            }
        }

    std::vector<uint32_t> CEVulkanShaderManager::ReadSPIRVFile ( const std::string & filename )
        {
        std::ifstream file ( filename, std::ios::ate | std::ios::binary );

        if (!file.is_open ())
            {
            throw std::runtime_error ( "Failed to open shader file: " + filename );
            }

        size_t fileSize = ( size_t ) file.tellg ();
        std::vector<uint32_t> buffer ( fileSize / sizeof ( uint32_t ) );

        file.seekg ( 0 );
        file.read ( reinterpret_cast< char * >( buffer.data () ), fileSize );
        file.close ();

        return buffer;
        }

    bool CEVulkanShaderManager::CompileGLSLToSPIRV ( const std::string & sourcePath, const std::string & outputPath )
        {
            // This would invoke an external compiler like glslangValidator
            // For now, we assume pre-compiled SPIR-V files
        CE_CORE_WARN ( "GLSL compilation not implemented, using pre-compiled SPIR-V" );
        return false;
        }

    VkShaderModule CEVulkanShaderManager::CreateShaderModule ( const std::vector<uint32_t> & code )
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for shader module creation" );
            }

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size () * sizeof ( uint32_t );
        createInfo.pCode = code.data ();

        VkShaderModule shaderModule;
        VkResult result = vkCreateShaderModule ( m_Context->GetDevice ()->GetDevice (), &createInfo, nullptr, &shaderModule );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create shader module" );
            }

        return shaderModule;
        }
    }