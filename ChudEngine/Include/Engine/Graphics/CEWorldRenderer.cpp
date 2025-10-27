// CEWorldRenderer.cpp (обновленная версия)
#include "CEWorldRenderer.hpp"
#include "Core/Logger.h"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Graphics/Vulkan/Core/CEVulkanRenderer.hpp" 
#include "Graphics/Vulkan/Pipelines/CEStaticMeshPipeline.hpp"
#include <set>

namespace CE
	{
	CEWorldRenderer::CEWorldRenderer ( CEVulkanRenderer * renderer )
		: m_Renderer ( renderer )
		{
		CE_DEBUG ( "CEWorldRenderer created" );
		}

	CEWorldRenderer::~CEWorldRenderer ()
		{
		CE_DEBUG ( "CEWorldRenderer destroyed" );
		}

	std::vector<CEMeshComponent *> CEWorldRenderer::GatherMeshComponents ()
		{
		std::vector<CEMeshComponent *> meshComponents;

		if (!m_World)
			{
			CE_DEBUG ( "No world set for mesh gathering" );
			return meshComponents;
			}

			// Получаем все меш компоненты через мир
		auto components = m_World->GetComponentsOfType<CEMeshComponent> ();
		CE_DEBUG ( "Found {} mesh components in world", components.size () );

		for (auto * component : components)
			{
			if (component)
				{
				meshComponents.push_back ( component );
				}
			}

		return meshComponents;
		}

	void CEWorldRenderer::Render ( VkCommandBuffer commandBuffer )
		{
		if (!m_World || !m_Renderer)
			{
			CE_DEBUG ( "World or renderer not set for rendering" );
			return;
			}

		auto meshComponents = GatherMeshComponents ();
		CE_DEBUG ( "Rendering {} mesh components", meshComponents.size () );

		// Получаем статик меш пайплайн
		auto pipelineManager = m_Renderer->GetPipelineManager ();
		if (!pipelineManager)
			{
			CE_ERROR ( "Pipeline manager not available" );
			return;
			}

		auto staticMeshPipeline = pipelineManager->GetStaticMeshPipeline ();
		if (!staticMeshPipeline)
			{
			CE_ERROR ( "Static mesh pipeline not available" );
			return;
			}

			// Биндим пайплайн один раз для всех мешей
		staticMeshPipeline->Bind ( commandBuffer );

		for (auto * meshComponent : meshComponents)
			{
			if (meshComponent)
				{
				RenderMeshComponent ( meshComponent, commandBuffer, staticMeshPipeline );
				}
			}
		}

	void CEWorldRenderer::RenderMeshComponent ( CEMeshComponent * meshComponent,
												VkCommandBuffer commandBuffer,
												CEStaticMeshPipeline * pipeline )
		{
		if (!EnsureMeshBuffersCreated ( meshComponent ))
			{
			CE_ERROR ( "CANNOT RENDER: Buffers not created for '{}'", meshComponent->GetName () );
			return;
			}

			// GET TRANSFORM MATRICES
		auto * owner = meshComponent->GetOwner ();
		auto * transform = owner ? owner->GetTransform () : nullptr;

		if (!transform)
			{
			CE_ERROR ( "No transform for mesh component '{}'", meshComponent->GetName () );
			return;
			}

	

   //   // Calculate matrices
		Math::Matrix4 modelMatrix = transform->GetWorldTransform ();
		Math::Matrix4 viewMatrix = m_Renderer->GetViewMatrix ();
		Math::Matrix4 projectionMatrix = m_Renderer->GetProjectionMatrix ();

		// Для column-major: ViewProjection = Projection * View
		Math::Matrix4 viewProjectionMatrix = projectionMatrix * viewMatrix;

		// Prepare push constants - БЕЗ транспонирования!
		MatrixPushConstants pushConstants {};
		pushConstants.modelMatrix = modelMatrix; // column-major как есть
		pushConstants.viewProjectionMatrix = viewProjectionMatrix; // column-major как есть

		// Диагностика
		CE_DEBUG ( "=== Matrix Debug (Column-Major) ===" );
		CE_DEBUG ( "Push constants size: {} bytes", sizeof ( MatrixPushConstants ) );

		
		
		if (transform)
			{
			CE_DEBUG ( "Raw Position: ({:.2f}, {:.2f}, {:.2f})",
					   transform->GetPosition ().x,
					   transform->GetPosition ().y,
					   transform->GetPosition ().z );
			}

			// Выводим матрицы
		modelMatrix.DebugPrint ( "Model Matrix" );
		viewMatrix.DebugPrint ( "View Matrix" );
		projectionMatrix.DebugPrint ( "Projection Matrix" );
		viewProjectionMatrix.DebugPrint ( "ViewProjection Matrix" );

		// Проверяем результат трансформации
		Math::Vector4 testVertex ( 0, 0, 0, 1 ); // Тестовая вершина в локальных координатах
		Math::Vector4 worldPos = modelMatrix * testVertex;
		Math::Vector4 clipPos = viewProjectionMatrix * worldPos;

		CE_DEBUG ( "Transform Test: Local(0,0,0) -> World({:.2f}, {:.2f}, {:.2f}) -> Clip({:.2f}, {:.2f}, {:.2f})",
				   worldPos.x, worldPos.y, worldPos.z,
				   clipPos.x, clipPos.y, clipPos.z );

		CE_DEBUG ( "=== VERTEX CLIP SPACE ANALYSIS ===" );
		auto vertices = meshComponent->GetVertices ();
		for (size_t i = 0; i < vertices.size (); ++i)
			{
			Math::Vector4 localPos ( vertices[ i ].Position, 1.0f );
			Math::Vector4 worldPos = modelMatrix * localPos;
			Math::Vector4 clipPos = viewProjectionMatrix * worldPos;

			CE_DEBUG ( "Vertex {}: Local({:.2f}, {:.2f}, {:.2f}) -> Clip({:.2f}, {:.2f}, {:.2f})",
					   i,
					   vertices[ i ].Position.x, vertices[ i ].Position.y, vertices[ i ].Position.z,
					   clipPos.x, clipPos.y, clipPos.z );

			  // Проверка видимости
			bool visible = ( std::abs ( clipPos.x ) <= 1.0f && std::abs ( clipPos.y ) <= 1.0f &&
							 clipPos.z >= -1.0f && clipPos.z <= 1.0f );
			if (!visible)
				{
				CE_DEBUG ( "  Vertex {} is OUTSIDE clip space!", i );
				}
			}
	
		// PASS MATRICES TO SHADER via push constants
		vkCmdPushConstants (
			commandBuffer,
			pipeline->GetLayout (),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof ( MatrixPushConstants ),
			&pushConstants
		);

		CE_DEBUG ( "Push constants applied successfully" );

		// Now render the mesh
		meshComponent->Render ( commandBuffer );
		}

	bool CEWorldRenderer::EnsureMeshBuffersCreated ( CEMeshComponent * meshComponent )
		{
		if (!meshComponent || !m_Renderer)
			{
			CE_ERROR ( "EnsureMeshBuffersCreated: Invalid meshComponent or renderer" );
			return false;
			}

			// ДЕТАЛЬНАЯ ДИАГНОСТИКА
		CE_DEBUG ( "=== Ensuring buffers for mesh: {} ===", meshComponent->GetName () );
		CE_DEBUG ( "  Vertex count: {}", meshComponent->GetVertexCount () );
		CE_DEBUG ( "  Index count: {}", meshComponent->GetIndexCount () );
		CE_DEBUG ( "  IsValid: {}", meshComponent->IsValid () );
		

		// Создаем vertex buffer если нужно
		if (!meshComponent->IsValid () && meshComponent->GetVertexCount () > 0)
			{
			CE_DEBUG ( "  Creating vertex buffer..." );
			if (!meshComponent->CreateVertexBuffer ( m_Renderer ))
				{
				CE_ERROR ( "  FAILED to create vertex buffer for '{}'", meshComponent->GetName () );
				return false;
				}
			CE_DEBUG ( "  Vertex buffer created successfully" );
			}

			// Создаем index buffer если есть индексы
		if (meshComponent->HasIndices () && !meshComponent->GetIndexBuffer ())
			{
			CE_DEBUG ( "  Creating index buffer..." );
			if (!meshComponent->CreateIndexBuffer ( m_Renderer ))
				{
				CE_ERROR ( "  FAILED to create index buffer for '{}'", meshComponent->GetName () );
				// Продолжаем без индексного буфера
				}
			else
				{
				CE_DEBUG ( "  Index buffer created successfully" );
				}
			}

		bool result = meshComponent->IsValid ();
		CE_DEBUG ( "  Final IsValid: {}", result );
		return result;
		}
	}