#pragma once
#include <string>
#include <vector>

#include <unordered_map>

namespace CE
{
    class ShaderCompiler
    {
    public:
        struct ShaderCompileResult
        {
            bool success;
            std::string message;
            std::string compiledPath;
        };

        ShaderCompiler();
        ~ShaderCompiler() = default;

        bool CompileAllShaders(const std::string& inputDir, const std::string& outputDir);
        ShaderCompileResult CompileShader(const std::string& inputPath, const std::string& outputPath);

        static std::string FindVulkanSDK();
        static std::string FindGlslangValidator();
        static std::string GetShaderStage(const std::string& extension);

        void SetOptimizationLevel(int level) { m_optimizationLevel = level; }
        void SetGenerateDebugInfo(bool debug) { m_generateDebugInfo = debug; }

    private:
        int m_optimizationLevel;
        bool m_generateDebugInfo;

        std::string BuildCompileCommand(const std::string& inputPath, const std::string& outputPath);
    };
}