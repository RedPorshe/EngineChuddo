#include "ShaderCompiler.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace CE
    {
    ShaderCompiler::ShaderCompiler ()
        : m_optimizationLevel ( 0 )
        , m_generateDebugInfo ( false )
        {
        }

    bool ShaderCompiler::CompileAllShaders ( const std::string & inputDir, const std::string & outputDir )
        {
        if (!fs::exists ( inputDir ))
            {
            std::cout << "ERROR: Input directory does not exist: " << inputDir << std::endl;
            return false;
            }

        std::string glslangPath = FindGlslangValidator ();
        if (glslangPath.empty ())
            {
            std::cout << "ERROR: glslangValidator not found! Please install Vulkan SDK." << std::endl;
            return false;
            }

        std::cout << "Using glslangValidator: " << glslangPath << std::endl;
        std::cout << "Compiling shaders from: " << inputDir << std::endl;
        std::cout << "Output directory: " << outputDir << std::endl;

        int compiledCount = 0;
        int errorCount = 0;

        // Рекурсивный обход всех файлов
        try
            {
            for (const auto & entry : fs::recursive_directory_iterator ( inputDir ))
                {
                if (!entry.is_regular_file ()) continue;

                fs::path inputPath = entry.path ();
                std::string extension = inputPath.extension ().string ();

                // Проверяем поддерживаемые расширения
                if (extension != ".vert" && extension != ".frag" &&
                     extension != ".geom" && extension != ".tesc" &&
                     extension != ".tese" && extension != ".comp")
                    {
                    continue;
                    }

                    // Создаем соответствующий выходной путь с двойным расширением
                fs::path relativePath = fs::relative ( inputPath, inputDir );
                fs::path outputPath = fs::path ( outputDir ) / relativePath;
                outputPath += ".spv"; // simple.vert → simple.vert.spv

                // Создаем директорию для выходного файла
                fs::create_directories ( outputPath.parent_path () );

                // Компилируем шейдер
                auto result = CompileShader ( inputPath.string (), outputPath.string () );
                if (result.success)
                    {
                    std::cout << "COMPILED: " << relativePath.string () << std::endl;
                    compiledCount++;
                    }
                else
                    {
                    std::cout << "FAILED: " << relativePath.string () << " - " << result.message << std::endl;
                    errorCount++;
                    }
                }
            }
            catch (const std::exception & e)
                {
                std::cout << "ERROR during file traversal: " << e.what () << std::endl;
                return false;
                }

            std::cout << "Compilation completed: " << compiledCount << " succeeded, "
                << errorCount << " failed" << std::endl;
            return errorCount == 0;
        }

    ShaderCompiler::ShaderCompileResult ShaderCompiler::CompileShader (
        const std::string & inputPath, const std::string & outputPath )
        {
        ShaderCompileResult result;

        // Проверяем существование исходного файла
        if (!fs::exists ( inputPath ))
            {
            result.success = false;
            result.message = "Source file does not exist: " + inputPath;
            return result;
            }

            // Создаем директорию для выходного файла
        fs::create_directories ( fs::path ( outputPath ).parent_path () );

        std::string command = BuildCompileCommand ( inputPath, outputPath );

        std::cout << "Executing: " << command << std::endl;

        // Выполняем команду компиляции
        int compileResult = std::system ( command.c_str () );

        if (compileResult == 0 && fs::exists ( outputPath ))
            {
            result.success = true;
            result.message = "Success";
            result.compiledPath = outputPath;
            }
        else
            {
            result.success = false;
            result.message = "Compilation failed with code: " + std::to_string ( compileResult );
            }

        return result;
        }

    std::string ShaderCompiler::BuildCompileCommand ( const std::string & inputPath, const std::string & outputPath )
        {
        std::string glslangPath = FindGlslangValidator ();
        std::string stage = GetShaderStage ( inputPath.substr ( inputPath.find_last_of ( '.' ) ) );

        std::string command = glslangPath;
        command += " -V"; // Vulkan SPIR-V
        command += " --target-env vulkan1.2";

        // Без оптимизации - флаг -O вызывает ошибку
        if (m_generateDebugInfo)
            {
            command += " -g";
            }

        if (!stage.empty ())
            {
            command += " -S " + stage;
            }

        command += " -o " + outputPath;
        command += " " + inputPath;

        return command;
        }

    std::string ShaderCompiler::FindVulkanSDK ()
        {
            // Проверяем переменные окружения
        const char * envVars [] = { "VULKAN_SDK", "VK_SDK_PATH" };

        for (const char * envVar : envVars)
            {
            char * value = nullptr;
            size_t size = 0;
            if (_dupenv_s ( &value, &size, envVar ) == 0 && value != nullptr)
                {
                std::string result ( value );
                free ( value );
                if (!result.empty ())
                    {
                        // Нормализуем путь - убираем завершающий слеш если есть
                    if (result.back () == '\\')
                        {
                        result.pop_back ();
                        }
                    return result;
                    }
                }
            }

        return "";
        }

    std::string ShaderCompiler::FindGlslangValidator ()
        {
        std::string vulkanSDK = FindVulkanSDK ();
        if (!vulkanSDK.empty ())
            {
                // Убираем двойные слеши и нормализуем путь
            std::string validatorPath = vulkanSDK;

            // Заменяем двойные слеши на одинарные
            size_t pos = 0;
            while (( pos = validatorPath.find ( "\\\\", pos ) ) != std::string::npos)
                {
                validatorPath.replace ( pos, 2, "\\" );
                }

            validatorPath += "\\Bin\\glslangValidator.exe";

            // Проверяем существование файла с помощью filesystem
            if (fs::exists ( validatorPath ))
                {
                return validatorPath;
                }
            }

            // Проверяем PATH
        std::string command = "where glslangValidator";
        FILE * pipe = _popen ( command.c_str (), "r" );
        if (pipe)
            {
            char buffer[ 512 ];
            if (fgets ( buffer, sizeof ( buffer ), pipe ))
                {
                _pclose ( pipe );
                std::string path ( buffer );
                path.erase ( path.find_last_not_of ( " \t\n\r\f\v" ) + 1 );

                // Проверяем существование
                if (fs::exists ( path ))
                    {
                    return path;
                    }
                }
            _pclose ( pipe );
            }

        return "";
        }

    std::string ShaderCompiler::GetShaderStage ( const std::string & extension )
        {
        static const std::unordered_map<std::string, std::string> stageMap = {
            {".vert", "vert"}, {".frag", "frag"}, {".geom", "geom"},
            {".tesc", "tesc"}, {".tese", "tese"}, {".comp", "comp"},
            {".mesh", "mesh"}, {".task", "task"}, {".rgen", "rgen"},
            {".rint", "rint"}, {".rahit", "rahit"}, {".rchit", "rchit"},
            {".rmiss", "rmiss"}, {".rcall", "rcall"}
            };

        auto it = stageMap.find ( extension );
        return it != stageMap.end () ? it->second : "";
        }
    }