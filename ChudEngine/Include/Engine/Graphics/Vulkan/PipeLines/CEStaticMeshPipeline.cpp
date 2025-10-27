// Runtime/Renderer/Vulkan/Pipelines/CEStaticMeshPipeline.cpp
#include "CEStaticMeshPipeline.hpp"

namespace CE
    {
    CEStaticMeshPipeline::CEStaticMeshPipeline ( CEVulkanContext * context, CEVulkanShaderManager * shaderManager )
        : CEVulkanBasePipeline ( context, shaderManager, PipelineConfig { "StaticMesh" } )
        {
        // ������������� ����������� ��� StaticMeshPipeline �������
       // SetVertexShader ( "Resources/Shaders/Vulkan/static.vert" );
      //  SetFragmentShader ( "Resources/Shaders/Vulkan/static.frag" );
        }
    }