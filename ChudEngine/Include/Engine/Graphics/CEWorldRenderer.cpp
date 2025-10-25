// CEWorldRenderer.cpp (����������� ������)
#include "CEWorldRenderer.hpp"
#include "Core/Logger.h"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Graphics/Vulkan/CEVulkanRenderer.hpp" 
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

			// �������� ��� ��� ���������� ����� ���
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

		// �������� ������ ��� ��������
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

			// ������ �������� ���� ��� ��� ���� �����
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

			// Calculate model matrix
		Math::Matrix4 modelMatrix = transform->GetWorldTransform ();

		// Get view/projection from renderer (you'll need to add these getters)
		Math::Matrix4 viewMatrix = m_Renderer->GetViewMatrix ();
		Math::Matrix4 projectionMatrix = m_Renderer->GetProjectionMatrix ();
		Math::Matrix4 viewProjectionMatrix = projectionMatrix * viewMatrix;

		// Prepare push constants
		MatrixPushConstants pushConstants {};
		pushConstants.modelMatrix = modelMatrix;
		pushConstants.viewProjectionMatrix = viewProjectionMatrix;

		// PASS MATRICES TO SHADER via push constants
		vkCmdPushConstants (
			commandBuffer,
			pipeline->GetLayout (),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof ( MatrixPushConstants ),
			&pushConstants
		);

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

			// ��������� �����������
		CE_DEBUG ( "=== Ensuring buffers for mesh: {} ===", meshComponent->GetName () );
		CE_DEBUG ( "  Vertex count: {}", meshComponent->GetVertexCount () );
		CE_DEBUG ( "  Index count: {}", meshComponent->GetIndexCount () );
		CE_DEBUG ( "  IsValid: {}", meshComponent->IsValid () );
		

		// ������� vertex buffer ���� �����
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

			// ������� index buffer ���� ���� �������
		if (meshComponent->HasIndices () && !meshComponent->GetIndexBuffer ())
			{
			CE_DEBUG ( "  Creating index buffer..." );
			if (!meshComponent->CreateIndexBuffer ( m_Renderer ))
				{
				CE_ERROR ( "  FAILED to create index buffer for '{}'", meshComponent->GetName () );
				// ���������� ��� ���������� ������
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