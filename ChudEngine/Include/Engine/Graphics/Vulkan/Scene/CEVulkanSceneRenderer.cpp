#include "Graphics/Vulkan/Scene/CEVulkanSceneRenderer.hpp"
#include "Graphics/Vulkan/CEVulkanRenderer.hpp"
#include "Core/CEObject/CEWorld.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Graphics/Vulkan/Pipelines/CEStaticMeshPipeline.hpp"
#include "Graphics/Vulkan/Pipelines/CESkeletalMeshPipeline.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanSceneRenderer::CEVulkanSceneRenderer ( CEVulkanRenderer * renderer )
        : m_World ( nullptr )
        , m_Renderer ( renderer )
        , m_Initialized ( false )
        {
        }

    CEVulkanSceneRenderer::~CEVulkanSceneRenderer ()
        {
        Shutdown ();
        }

    bool CEVulkanSceneRenderer::Initialize ()
        {
        if (m_Initialized)
            {
            return true;
            }

            // Scene renderer doesn't need much initialization - it uses existing renderer components
        m_Initialized = true;
        CE_CORE_DEBUG ( "Scene renderer initialized" );
        return true;
        }

    void CEVulkanSceneRenderer::Shutdown ()
        {
        m_World = nullptr;
        m_Renderer = nullptr;
        m_Initialized = false;
        CE_CORE_DEBUG ( "Scene renderer shut down" );
        }

    void CEVulkanSceneRenderer::Render ( VkCommandBuffer commandBuffer )
        {
        if (!m_World || !m_Renderer)
            {
            return;
            }

            // Update scene (animation, transforms, etc.)
            // In a real implementation, this would use deltaTime
        Update ( 0.016f ); // Assume 60 FPS for now

        // Render different types of objects
        RenderStaticMeshes ( commandBuffer );
        RenderSkeletalMeshes ( commandBuffer );
        RenderLights ( commandBuffer );
        RenderDebug ( commandBuffer );
        }

    void CEVulkanSceneRenderer::Update ( float deltaTime )
        {
        if (!m_World) return;

        // Update world state (animations, physics, etc.)
        // This would iterate through all objects and update them
        CE_CORE_TRACE ( "Updating scene with deltaTime: {}", deltaTime );

        // Example: Update animations, transform hierarchies, etc.
        // m_World->Update(deltaTime);
        }

    void CEVulkanSceneRenderer::RenderStaticMeshes ( VkCommandBuffer commandBuffer )
        {
        if (!m_Renderer || !m_World) return;

        auto pipeline = m_Renderer->GetPipelineManager ()->GetPipelineAs<CEStaticMeshPipeline> ( PipelineType::StaticMesh );
        if (!pipeline)
            {
            CE_CORE_WARN ( "Static mesh pipeline not available" );
            return;
            }

        pipeline->Bind ( commandBuffer );

        // Gather and render all static mesh components
        auto meshComponents = GatherMeshComponents ();

        for (auto meshComponent : meshComponents)
            {
            if (meshComponent && meshComponent->IsStatic ())
                {
                RenderMeshComponent ( meshComponent, commandBuffer, pipeline );
                }
            }
        }

    void CEVulkanSceneRenderer::RenderSkeletalMeshes ( VkCommandBuffer commandBuffer )
        {
        if (!m_Renderer || !m_World) return;

        auto pipeline = m_Renderer->GetPipelineManager ()->GetPipelineAs<CESkeletalMeshPipeline> ( PipelineType::SkeletalMesh );
        if (!pipeline)
            {
            CE_CORE_WARN ( "Skeletal mesh pipeline not available" );
            return;
            }

        pipeline->Bind ( commandBuffer );

        // Gather and render all skeletal mesh components
        auto meshComponents = GatherMeshComponents ();

        for (auto meshComponent : meshComponents)
            {
            if (meshComponent && !meshComponent->IsStatic ())
                {
                RenderMeshComponent ( meshComponent, commandBuffer, nullptr ); // Would use skeletal pipeline
                }
            }
        }

    void CEVulkanSceneRenderer::RenderLights ( VkCommandBuffer commandBuffer )
        {
        if (!m_Renderer || !m_World) return;

        auto pipeline = m_Renderer->GetPipelineManager ()->GetPipelineAs<CELightPipeline> ( PipelineType::Light );
        if (!pipeline)
            {
            CE_CORE_WARN ( "Light pipeline not available" );
            return;
            }

        pipeline->Bind ( commandBuffer );

        // This would iterate through all light components in the world
        // and render them as light volumes or deferred lighting passes

        CE_CORE_TRACE ( "Rendering lights" );
        // Implementation would depend on lighting technique (forward vs deferred)
        }

    void CEVulkanSceneRenderer::RenderDebug ( VkCommandBuffer commandBuffer )
        {
            // Debug rendering is handled by CEVulkanDebugRenderer
            // This method could add scene-specific debug information
        CE_CORE_TRACE ( "Rendering scene debug information" );
        }

    std::vector<CEMeshComponent *> CEVulkanSceneRenderer::GatherMeshComponents ()
        {
        std::vector<CEMeshComponent *> meshComponents;

        if (!m_World) return meshComponents;

        // This would iterate through all objects in the world and collect mesh components
        // For now, return empty list - implementation depends on your scene graph

        // Example:
        // for (auto& object : m_World->GetObjects()) {
        //     if (auto meshComp = object->GetComponent<CEMeshComponent>()) {
        //         meshComponents.push_back(meshComp);
        //     }
        // }

        return meshComponents;
        }

    void CEVulkanSceneRenderer::RenderMeshComponent ( CEMeshComponent * meshComponent,
                                                      VkCommandBuffer commandBuffer,
                                                      CEStaticMeshPipeline * pipeline )
        {
        if (!meshComponent || !pipeline) return;

        // Ensure mesh buffers are created and ready
        if (!EnsureMeshBuffersCreated ( meshComponent ))
            {
            CE_CORE_WARN ( "Mesh buffers not ready for rendering" );
            return;
            }

            // Set push constants for transformation matrices
        MatrixPushConstants pushConstants;
        pushConstants.modelMatrix = meshComponent->GetWorldTransform ();
        pushConstants.viewProjectionMatrix = m_Renderer->GetViewMatrix () * m_Renderer->GetProjectionMatrix ();

        vkCmdPushConstants (
            commandBuffer,
            pipeline->GetLayout (),
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof ( MatrixPushConstants ),
            &pushConstants
        );

        // Bind vertex and index buffers
        // This would use the mesh component's Vulkan buffers
        // vkCmdBindVertexBuffers(...);
        // vkCmdBindIndexBuffer(...);

        // Draw the mesh
        // vkCmdDrawIndexed(...);

        CE_CORE_TRACE ( "Rendered mesh component" );
        }

    bool CEVulkanSceneRenderer::EnsureMeshBuffersCreated ( CEMeshComponent * meshComponent )
        {
        if (!meshComponent) return false;

        // Check if the mesh component already has Vulkan buffers created
        // If not, create them using the resource manager

        // This would involve:
        // 1. Checking if buffers exist
        // 2. Creating vertex buffer from mesh data
        // 3. Creating index buffer from mesh data
        // 4. Uploading data to GPU

        return true; // Placeholder
        }
    }