#include "Graphics/Vulkan/Managers/CEVulkanShaderManager.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "ShaderCompiler.h"
#include <fstream>
#include <filesystem>

namespace CE
    {
    CEVulkanShaderManager::CEVulkanShaderManager ( CEVulkanContext * context )
        : m_Context ( context )
        {
        CE_CORE_DEBUG ( "Vulkan shader manager created" );
        }

    CEVulkanShaderManager::~CEVulkanShaderManager ()
        {
        Cleanup ();
        }

    std::shared_ptr<CEVulkanShaderManager::ShaderModule>
        CEVulkanShaderManager::LoadShader ( const std::string & filename, VkShaderStageFlagBits stage )
        {
        CE_CORE_DEBUG ( "Loading shader: {} (stage: {})", filename, static_cast< int >( stage ) );

        try
            {
            auto code = ReadSPIRVFile ( filename );
            if (code.empty ())
                {
                CE_CORE_ERROR ( "Failed to read shader file: {}", filename );
                return nullptr;
                }

            auto shaderModule = CreateShaderModule ( code );
            if (shaderModule == VK_NULL_HANDLE)
                {
                CE_CORE_ERROR ( "Failed to create shader module for: {}", filename );
                return nullptr;
                }

            auto shader = std::make_shared<ShaderModule> ();
            shader->module = shaderModule;
            shader->code = std::move ( code );
            shader->path = filename;
            shader->stage = stage;

            m_ShaderModules.push_back ( shader );
            CE_CORE_DEBUG ( "Shader loaded successfully: {}", filename );
            return shader;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Exception loading shader {}: {}", filename, e.what () );
                return nullptr;
                }
        }

    std::shared_ptr<CEVulkanShaderManager::ShaderModule>
        CEVulkanShaderManager::CompileAndLoadShader ( const std::string & sourcePath, VkShaderStageFlagBits stage )
        {
        if (!std::filesystem::exists ( sourcePath ))
            {
            CE_CORE_ERROR ( "Shader source file not found: {}", sourcePath );
            return nullptr;
            }

            // Определяем выходной путь
        std::string outputPath = sourcePath + ".spv";

        // Проверяем необходимость перекомпиляции
        bool needsCompilation = true;
        if (std::filesystem::exists ( outputPath ))
            {
            auto sourceTime = std::filesystem::last_write_time ( sourcePath );
            auto outputTime = std::filesystem::last_write_time ( outputPath );
            needsCompilation = ( sourceTime > outputTime );
            }

            // Компилируем если нужно
        if (needsCompilation && !CompileGLSLToSPIRV ( sourcePath, outputPath ))
            {
            CE_CORE_ERROR ( "Failed to compile shader: {}", sourcePath );
            return nullptr;
            }

            // Загружаем скомпилированный шейдер
        return LoadShader ( outputPath, stage );
        }

    void CEVulkanShaderManager::ReloadAllShaders ()
        {
        CE_CORE_DEBUG ( "Reloading all shaders..." );

        // Сохраняем информацию о шейдерах
        struct ShaderInfo
            {
            std::string path;
            VkShaderStageFlagBits stage;
            bool isCompiled;
            };

        std::vector<ShaderInfo> shaderInfos;
        for (const auto & shader : m_ShaderModules)
            {
            shaderInfos.push_back ( { shader->path, shader->stage,
                                 shader->path.find ( ".spv" ) == std::string::npos } );
            }

            // Очищаем текущие шейдеры
        Cleanup ();

        // Перезагружаем шейдеры
        for (const auto & info : shaderInfos)
            {
            if (info.isCompiled)
                {
                CompileAndLoadShader ( info.path, info.stage );
                }
            else
                {
                LoadShader ( info.path, info.stage );
                }
            }

        CE_CORE_DEBUG ( "All shaders reloaded" );
        }

    void CEVulkanShaderManager::DestroyShaderModule ( std::shared_ptr<ShaderModule> shaderModule )
        {
        if (shaderModule && shaderModule->module != VK_NULL_HANDLE)
            {
            vkDestroyShaderModule ( m_Context->GetDevice ()->GetDevice(), shaderModule->module, nullptr);
            shaderModule->module = VK_NULL_HANDLE;
            }

            // Удаляем из списка
        m_ShaderModules.erase (
            std::remove ( m_ShaderModules.begin (), m_ShaderModules.end (), shaderModule ),
            m_ShaderModules.end ()
        );
        }

    void CEVulkanShaderManager::Cleanup ()
        {
        for (auto & shader : m_ShaderModules)
            {
            if (shader->module != VK_NULL_HANDLE)
                {
                vkDestroyShaderModule ( m_Context->GetDevice (), shader->module, nullptr );
                shader->module = VK_NULL_HANDLE;
                }
            }
        m_ShaderModules.clear ();
        CE_CORE_DEBUG ( "Shader manager cleanup complete" );
        }

    VkShaderStageFlagBits CEVulkanShaderManager::GetShaderStageFromExtension ( const std::string & filename )
        {
        if (filename.find ( ".vert" ) != std::string::npos) return VK_SHADER_STAGE_VERTEX_BIT;
        if (filename.find ( ".frag" ) != std::string::npos) return VK_SHADER_STAGE_FRAGMENT_BIT;
        if (filename.find ( ".geom" ) != std::string::npos) return VK_SHADER_STAGE_GEOMETRY_BIT;
        if (filename.find ( ".tesc" ) != std::string::npos) return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (filename.find ( ".tese" ) != std::string::npos) return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if (filename.find ( ".comp" ) != std::string::npos) return VK_SHADER_STAGE_COMPUTE_BIT;

        CE_CORE_WARN ( "Unknown shader stage for file: {}", filename );
        return VK_SHADER_STAGE_ALL;
        }

    std::vector<uint32_t> CEVulkanShaderManager::ReadSPIRVFile ( const std::string & filename )
        {
        std::ifstream file ( filename, std::ios::ate | std::ios::binary );
        if (!file.is_open ())
            {
            throw std::runtime_error ( "Failed to open file: " + filename );
            }

        size_t fileSize = ( size_t ) file.tellg ();
        std::vector<char> buffer ( fileSize );

        file.seekg ( 0 );
        file.read ( buffer.data (), fileSize );
        file.close ();

        if (!file)
            {
            throw std::runtime_error ( "Failed to read file: " + filename );
            }

            // Проверяем SPIR-V magic number
        if (buffer.size () >= 4)
            {
            uint32_t magic = *reinterpret_cast< uint32_t * >( buffer.data () );
            if (magic != 0x07230203)
                {
                throw std::runtime_error ( "Invalid SPIR-V magic number" );
                }
            }

            // Конвертируем в uint32_t
        std::vector<uint32_t> spirv ( buffer.size () / sizeof ( uint32_t ) );
        memcpy ( spirv.data (), buffer.data (), buffer.size () );

        CE_CORE_DEBUG ( "Loaded SPIR-V: {} ({} bytes)", filename, buffer.size () );
        return spirv;
        }

    bool CEVulkanShaderManager::CompileGLSLToSPIRV ( const std::string & sourcePath, const std::string & outputPath )
        {
        CE_CORE_DEBUG ( "Compiling GLSL: {} -> {}", sourcePath, outputPath );

        CE::ShaderCompiler compiler;
        compiler.SetOptimizationLevel ( 0 );
        compiler.SetGenerateDebugInfo ( true );

        auto result = compiler.CompileShader ( sourcePath, outputPath );
        if (!result.success)
            {
            CE_CORE_ERROR ( "Shader compilation failed: {} - {}", sourcePath, result.message );
            return false;
            }

        CE_CORE_DEBUG ( "Shader compiled successfully: {}", sourcePath );
        return true;
        }

    VkShaderModule CEVulkanShaderManager::CreateShaderModule ( const std::vector<uint32_t> & code )
        {
        auto device = m_Context->GetDevice ();

        VkShaderModuleCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size () * sizeof ( uint32_t );
        createInfo.pCode = code.data ();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule ( device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create shader module" );
            return VK_NULL_HANDLE;
            }

        return shaderModule;
        }
    }