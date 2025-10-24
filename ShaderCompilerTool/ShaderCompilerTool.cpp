#include "ShaderCompiler.h"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main ( int argc, char * argv [] )
    {
    std::cout << "=== ChudEngine Shader Compiler Tool ===" << std::endl;

    // Устанавливаем рабочую директорию в корень решения
    std::string solutionDir = fs::current_path ().string ();

    // Если запускаем из папки bin/Debug, поднимаемся на уровень выше
    if (solutionDir.find ( "\\bin\\" ) != std::string::npos)
        {
        solutionDir = fs::path ( solutionDir ).parent_path ().parent_path ().string ();
        fs::current_path ( solutionDir );
        }

    std::cout << "Working directory: " << fs::current_path () << std::endl;

    // Определяем пути относительно корня решения
    std::string sourceDir = "Resources/Shaders";
    std::string outputDir = "bin/Resources/Shaders";

    // Парсим аргументы командной строки
    if (argc >= 3)
        {
        sourceDir = argv[ 1 ];
        outputDir = argv[ 2 ];
        }
    else if (argc == 2)
        {
        std::cout << "Usage: ShaderCompilerTool.exe [source_dir] [output_dir]" << std::endl;
        std::cout << "Using default paths..." << std::endl;
        }

        // Преобразуем в абсолютные пути
    sourceDir = fs::absolute ( sourceDir ).string ();
    outputDir = fs::absolute ( outputDir ).string ();

    std::cout << "Source directory: " << sourceDir << std::endl;
    std::cout << "Output directory: " << outputDir << std::endl;

    // Проверяем существование исходной директории
    if (!fs::exists ( sourceDir ))
        {
        std::cout << "ERROR: Source directory does not exist: " << sourceDir << std::endl;
        return 1;
        }

        // Компилируем шейдеры
    CE::ShaderCompiler compiler;
    compiler.SetOptimizationLevel ( 0 ); // Отключаем оптимизацию
    compiler.SetGenerateDebugInfo ( true ); // Отладочная информация

    bool success = compiler.CompileAllShaders ( sourceDir, outputDir );

    if (success)
        {
        std::cout << "Shader compilation completed successfully!" << std::endl;
        return 0;
        }
    else
        {
        std::cout << "Shader compilation failed!" << std::endl;
        return 1;
        }
    }