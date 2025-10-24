// Runtime/Renderer/Vulkan/CEVulkanRenderer.cpp
#include "CEVulkanRenderer.hpp"
#include "CEVulkanContext.hpp"
#include "CEVulkanSwapchain.hpp"
#include "CEVulkanSync.hpp"
#include "CEVulkanPipeline.hpp"
#include "CEVulkanCommandBuffer.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "Math/Matrix.hpp"
#include "Math/MathUtils.hpp" 
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
	{

	void CEVulkanRenderer::SetCameraParameters ( const Math::Vector3 & position,
												 const Math::Vector3 & target,
												 float fov )
		{
			// Сохраняем параметры камеры
		CameraPosition = position;
		CameraTarget = target;
		CameraFOV = fov;

		// Создаем матрицу вида (view matrix)
		Math::Vector3 up = Math::Vector3::UnitY;
		ViewMatrix = Math::Matrix4::LookAt ( position, target, up );

		// Создаем матрицу проекции
		float aspectRatio = static_cast< float >( Window->GetWidth () ) / static_cast< float >( Window->GetHeight () );
		ProjectionMatrix = Math::Matrix4::Perspective ( Math::ToRadians ( fov ), aspectRatio, 0.1f, 1000.0f );



		}

	void CEVulkanRenderer::SetCameraViewMatrix ( const Math::Matrix4 & viewMatrix )
		{
		ViewMatrix = viewMatrix;
		}

	void CEVulkanRenderer::SetCameraProjectionMatrix ( const Math::Matrix4 & projectionMatrix )
		{
		ProjectionMatrix = projectionMatrix;
		}


	CEVulkanRenderer::CEVulkanRenderer ()
		{

		CE_CORE_DEBUG ( "Vulkan renderer created" );
		}

	CEVulkanRenderer::~CEVulkanRenderer ()
		{
		Shutdown ();
		}

	bool CEVulkanRenderer::Initialize ( CEWindow * window )
		{
		if (Initialized)
			{
			CE_CORE_WARN ( "VulkanRenderer already initialized" );
			return false;
			}

		Window = window;
		CE_CORE_DEBUG ( "Initializing Vulkan renderer" );

		try
			{
				// Initialize context
			Context = std::make_unique<CEVulkanContext> ();
			if (!Context->Initialize ( window ))
				{
				CE_CORE_ERROR ( "Failed to initialize Vulkan context" );
				return false;
				}

				// Initialize swapchain
			Swapchain = std::make_unique<CEVulkanSwapchain> ( Context.get () );
			if (!Swapchain->Initialize ( window ))
				{
				CE_CORE_ERROR ( "Failed to initialize Vulkan swapchain" );
				return false;
				}

				// Initialize sync objects
			SyncManager = std::make_unique<CEVulkanSync> ( Context.get () );
			if (!SyncManager->Initialize ())
				{
				CE_CORE_ERROR ( "Failed to initialize Vulkan sync objects" );
				return false;
				}

				// Initialize pipeline
			Pipeline = std::make_unique<CEVulkanPipeline> ( Context.get () );
			if (!Pipeline->Initialize ( Swapchain->GetRenderPass () ))
				{
				CE_CORE_ERROR ( "Failed to initialize Vulkan pipeline" );
				return false;
				}

				// Initialize command buffer
			CommandBuffer = std::make_unique<CEVulkanCommandBuffer> ( Context.get () );
			if (!CommandBuffer->Initialize ())
				{
				CE_CORE_ERROR ( "Failed to initialize Vulkan command buffer" );
				return false;
				}

			Initialized = true;
			CE_CORE_DEBUG ( "Vulkan renderer initialized successfully" );
			return true;
			}
			catch (const std::exception & e)
				{
				CE_CORE_ERROR ( "Failed to initialize Vulkan renderer: {}", e.what () );
				return false;
				}
		}

	void CEVulkanRenderer::Shutdown ()
		{
		if (!Initialized) return;

		CE_CORE_DEBUG ( "Shutting down Vulkan renderer" );

		auto device = Context ? Context->GetDevice () : VK_NULL_HANDLE;
		if (device != VK_NULL_HANDLE)
			{
			vkDeviceWaitIdle ( device );
			}

			// Cleanup in reverse order of initialization
		if (CommandBuffer)
			{
			CommandBuffer->Shutdown ();
			CommandBuffer.reset ();
			}

		if (Pipeline)
			{
			Pipeline->Shutdown ();
			Pipeline.reset ();
			}

		if (SyncManager)
			{
			SyncManager->Shutdown ();
			SyncManager.reset ();
			}

		if (Swapchain)
			{
			Swapchain->Shutdown ();
			Swapchain.reset ();
			}

		if (Context)
			{
			Context->Shutdown ();
			Context.reset ();
			}

		Initialized = false;
		CE_CORE_DEBUG ( "Vulkan renderer shutdown complete" );
		}

	void CEVulkanRenderer::RenderFrame ()
		{
		if (!Initialized)
			{
			CE_CORE_WARN ( "Renderer not initialized" );
			return;
			}

		static int frameCount = 0;
		frameCount++;



		try
			{
		   // 1. Ждем завершения предыдущего кадра

			if (!SyncManager->WaitForFrameFence ())
				{
				CE_CORE_ERROR ( "Failed to wait for frame fence" );
				return;
				}

				// 2. Сбрасываем fence для текущего кадра

			if (!SyncManager->ResetFrameFence ())
				{
				CE_CORE_ERROR ( "Failed to reset frame fence" );
				return;
				}

				// 3. Получаем следующий image из swapchain

			VkResult acquireResult = Swapchain->AcquireNextImage ( SyncManager->GetImageAvailableSemaphore () );

			if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
				{
				CE_CORE_DEBUG ( "Swapchain out of date, recreating..." );
				OnWindowResized ();
				return;
				}
			else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
				{
				CE_CORE_ERROR ( "Failed to acquire swapchain image: {}", static_cast< int >( acquireResult ) );
				return;
				}

				// 4. Сбрасываем command buffer

			CommandBuffer->Reset ();

			if (!CommandBuffer->IsReadyForRecording ())
				{
				CE_CORE_ERROR ( "Command buffer is not ready for recording!" );
				return;
				}

				// 5. Записываем command buffer

			CommandBuffer->BeginRecording ();
			RecordCommandBuffer ( CommandBuffer->GetCommandBuffer (), Swapchain->GetCurrentImageIndex () );
			CommandBuffer->EndRecording ();

			// 6. Отправляем command buffer

			VkResult submitResult = Swapchain->SubmitCommandBuffer (
				CommandBuffer->GetCommandBuffer (),
				SyncManager->GetImageAvailableSemaphore (),
				SyncManager->GetRenderFinishedSemaphore (),
				SyncManager->GetInFlightFence ()
			);

			if (submitResult == VK_ERROR_OUT_OF_DATE_KHR || submitResult == VK_SUBOPTIMAL_KHR)
				{
				CE_CORE_DEBUG ( "Swapchain needs recreation after submit" );
				OnWindowResized ();
				}
			else if (submitResult != VK_SUCCESS)
				{
				CE_CORE_ERROR ( "Failed to submit command buffer: {}", static_cast< int >( submitResult ) );
				}

				// 7. Переходим к следующему кадру

			SyncManager->AdvanceFrame ();


			}
			catch (const std::exception & e)
				{
				CE_CORE_ERROR ( "Error during frame rendering: {}", e.what () );
				}
		}

	void CEVulkanRenderer::OnWindowResized ()
		{
		if (!Initialized) return;

		CE_CORE_DEBUG ( "Recreating swapchain due to window resize" );

		// Ждем завершения всех операций
		vkDeviceWaitIdle ( Context->GetDevice () );

		// Проверяем валидный размер окна
		int width, height;
		glfwGetFramebufferSize ( Window->GetNativeWindow (), &width, &height );

		while (width == 0 || height == 0)
			{
			glfwGetFramebufferSize ( Window->GetNativeWindow (), &width, &height );
			glfwWaitEvents ();

			// Если окно закрыто, выходим
			if (glfwWindowShouldClose ( Window->GetNativeWindow () ))
				return;
			}

			// Очищаем старый swapchain
		Swapchain->Shutdown ();

		// Пересоздаем
		if (!Swapchain->Initialize ( Window ))
			{
			CE_CORE_ERROR ( "Failed to recreate swapchain!" );
			return;
			}

			// Пересоздаем pipeline с новым render pass
		Pipeline->Shutdown ();
		if (!Pipeline->Initialize ( Swapchain->GetRenderPass () ))
			{
			CE_CORE_ERROR ( "Failed to recreate pipeline!" );
			return;
			}

		CE_CORE_DEBUG ( "Swapchain and pipeline recreated successfully" );
		}

	void CEVulkanRenderer::RenderWorld ( CEWorld * world )
		{
		if (!world || !Initialized) return;

		// Просто рендерим кадр
		RenderFrame ();

		// Расширенная отладочная информация
		auto actors = world->GetActors ();


		for (auto * actor : actors)
			{
			if (actor)
				{


				  // Проверяем меш компоненты через GetComponentsOfType
				auto meshComps = actor->GetComponentsOfType<CEMeshComponent> ();
				if (!meshComps.empty ())
					{


					for (auto * meshComp : meshComps)
						{
						if (meshComp->IsValid ())
							{

							}
						else
							{
							CE_DEBUG ( "    Mesh component exists but not valid" );
							}
						}
					}

					// Проверяем через GetComponent
				auto meshComp = actor->GetComponent<CEMeshComponent> ();
				if (meshComp)
					{

					}
				else
					{
					CE_DEBUG ( "  Actor '{}' has NO mesh component via GetComponent", actor->GetName () );
					}
				}
			}
		}

	void CEVulkanRenderer::RecordCommandBuffer ( VkCommandBuffer commandBuffer, uint32_t imageIndex )
		{
		if (Pipeline)
			{
			Pipeline->SetViewMatrix ( ViewMatrix );
			Pipeline->SetProjectionMatrix ( ProjectionMatrix );
			}

		// Begin render pass
		VkRenderPassBeginInfo renderPassInfo {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = Swapchain->GetRenderPass ();
		renderPassInfo.framebuffer = Swapchain->GetFramebuffers ()[ imageIndex ];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Swapchain->GetExtent ();

		CEArray<VkClearValue> clearValues;
		clearValues.Resize ( 1 );
		clearValues[ 0 ].color = { {0.2f, 0.2f, 0.8f, 1.0f} };

		renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.Size () );
		renderPassInfo.pClearValues = clearValues.RawData ();

		vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

		if (Pipeline && Pipeline->GetPipeline () != VK_NULL_HANDLE)
			{
			vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipeline () );
			}
		else
			{
			CE_ERROR ( "Graphics pipeline is not valid!" );
			return;
			}

		// Set dynamic viewport and scissor
		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( Swapchain->GetExtent ().width );
		viewport.height = static_cast< float >( Swapchain->GetExtent ().height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport ( commandBuffer, 0, 1, &viewport );

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent = Swapchain->GetExtent ();
		vkCmdSetScissor ( commandBuffer, 0, 1, &scissor );

		// РЕНДЕРИМ МЕШ КОМПОНЕНТЫ ИЗ АКТОРОВ
		if (CurrentApplication && CurrentApplication->GetWorld ())
			{
			auto world = CurrentApplication->GetWorld ();
			auto actors = world->GetActors ();

			bool renderedAnyMesh = false;
			for (auto * actor : actors)
				{
				if (actor)
					{
					RenderActor ( actor, commandBuffer );
					renderedAnyMesh = true;
					}
				}

			// Если не отрендерили ни одного меша, показываем fallback
			if (!renderedAnyMesh)
				{
				// Для fallback рендеринга нужно привязать дескрипторный сет с нулевым смещением
				VkDescriptorSet descriptorSet = Pipeline->GetDescriptorSet ( SyncManager->GetCurrentFrame () );
				if (descriptorSet != VK_NULL_HANDLE)
					{
					uint32_t dynamicOffset = 0;
					vkCmdBindDescriptorSets (
						commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						Pipeline->GetLayout (),
						0, 1, &descriptorSet,
						1, &dynamicOffset
					);
					}

				Pipeline->BindVertexBuffer ( commandBuffer );
				vkCmdDraw ( commandBuffer, 3, 1, 0, 0 );
				CE_DEBUG ( "Rendered fallback triangle (no valid meshes)" );
				}
			}
		else
			{
			// Fallback: render default triangle
			VkDescriptorSet descriptorSet = Pipeline->GetDescriptorSet ( SyncManager->GetCurrentFrame () );
			if (descriptorSet != VK_NULL_HANDLE)
				{
				uint32_t dynamicOffset = 0;
				vkCmdBindDescriptorSets (
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					Pipeline->GetLayout (),
					0, 1, &descriptorSet,
					1, &dynamicOffset
				);
				}

			Pipeline->BindVertexBuffer ( commandBuffer );
			vkCmdDraw ( commandBuffer, 3, 1, 0, 0 );
			CE_DEBUG ( "Rendered default triangle (no world)" );
			}

		// End render pass
		vkCmdEndRenderPass ( commandBuffer );
		}


	void CEVulkanRenderer::RenderActor ( CEActor * actor, VkCommandBuffer commandBuffer ) {
		if (!actor) return;

		auto meshComp = actor->GetComponent<CEMeshComponent> ();
		if (meshComp && meshComp->IsValid ())
			{
			// Получаем индекс объекта
			static std::unordered_map<CEActor *, uint32_t> objectIndices;
			static uint32_t nextObjectIndex = 0;

			uint32_t objectIndex;
			if (objectIndices.find ( actor ) == objectIndices.end ())
				{
				objectIndex = nextObjectIndex++;
				objectIndices[ actor ] = objectIndex;

				if (objectIndex >= Pipeline->GetMaxObjects ())
					{
					CE_CORE_ERROR ( "Too many objects! Maximum is {}", Pipeline->GetMaxObjects () );
					return;
					}
				}
			else
				{
				objectIndex = objectIndices[ actor ];
				}

				// Получаем матрицу трансформации
			Math::Matrix4 modelMatrix = meshComp->GetTransformMatrix ();

			// Обновляем динамические uniform buffers
			Pipeline->UpdateDynamicUniforms ( SyncManager->GetCurrentFrame (), objectIndex, modelMatrix );

			// Вычисляем динамическое смещение
			uint32_t dynamicOffset = objectIndex * static_cast< uint32_t >( Pipeline->GetDynamicAlignment () );

			// Привязываем descriptor set с динамическим смещением
			VkDescriptorSet descriptorSet = Pipeline->GetDescriptorSet ( SyncManager->GetCurrentFrame () );
			if (descriptorSet != VK_NULL_HANDLE)
				{
				vkCmdBindDescriptorSets (
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					Pipeline->GetLayout (),
					0, 1, &descriptorSet,
					1, &dynamicOffset  // ДИНАМИЧЕСКОЕ СМЕЩЕНИЕ
				);

				// Рендерим меш
				meshComp->Render ( commandBuffer );

				CE_DEBUG ( "Rendered actor '{}' as object {} with dynamic offset {}",
						   actor->GetName (), objectIndex, dynamicOffset );
				}
			else
				{
				CE_CORE_ERROR ( "Failed to get descriptor set for actor {}", actor->GetName () );
				}
			}
		}
	}