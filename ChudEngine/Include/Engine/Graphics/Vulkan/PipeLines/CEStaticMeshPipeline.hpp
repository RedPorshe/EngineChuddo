// Runtime/Renderer/Vulkan/Pipelines/CEStaticMeshPipeline.hpp
#pragma once
#include "../CEVulkanBasePipeline.hpp"
#include <memory>

namespace CE
    {
    class CEStaticMeshPipeline : public CEVulkanBasePipeline
        {
        public:
            CEStaticMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager );
            ~CEStaticMeshPipeline () override = default;

          

        private:
            // Специфичные для StaticMesh ресурсы
        };
    }