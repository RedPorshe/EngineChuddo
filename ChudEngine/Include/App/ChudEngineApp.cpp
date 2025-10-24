#include "ChudEngineApp.hpp"
#include "Core/CEObject/CEActor.hpp"
#include "Graphics/Vulkan/CEVulkanRenderer.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "../MatrixDebugUtils.hpp"
#include "Core/Logger.h"




void ChudEngineApp::SetupCamera ()
	{
	CameraPosition = CE::Math::Vector3 ( 0.0f, 0.0f, -2.0f ); // Камера смотрит с позиции (0,0,-2)
	CameraTarget = CE::Math::Vector3 ( 0.0f, 0.0f, 0.0f );   // Смотрит в центр
	CameraFOV = 60.0f; // 60 градусов

	// Устанавливаем параметры камеры в рендерер
	auto * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );
	if (vulkanRenderer)
		{
		vulkanRenderer->SetCameraParameters ( CameraPosition, CameraTarget, CameraFOV );
		CE_CORE_DEBUG ( "Camera setup: position=({}, {}, {}), target=({}, {}, {}), FOV={}",
						CameraPosition.x, CameraPosition.y, CameraPosition.z,
						CameraTarget.x, CameraTarget.y, CameraTarget.z,
						CameraFOV );
		CheckCommonMatrixProblems ();
		}
	}

class CTestActor : public CE::CEActor
	{
	public:
		CTestActor ( const std::string & name ) : CEActor ( name )
			{
			SetName ( name );

			// Создаем меш компонент через CreateComponent
			MeshComponent = CreateComponent<CE::CEMeshComponent> ();
			if (MeshComponent)
				{
				MeshComponent->SetName ( name + "_Mesh" );
				CE_DEBUG ( "Created mesh component for actor: {}", name );
				}
			else
				{
				CE_DEBUG ( "Failed to create mesh component for actor: {}", name );
				}
			}

		void BeginPlay () override
			{
			CE::CEActor::BeginPlay ();
			CE_DEBUG ( "CTestActor '{}' BeginPlay", GetName () );

			// Используем компонент трансформации для позиционирования
			auto * transform = GetTransform ();
			if (transform)
				{
				transform->SetPosition ( CE::Math::Vector3 ( .5f, .5f, -5.f ) );
				transform->SetScale ( 0.8f );
				}

// Устанавливаем вершины и индексы
			if (MeshComponent)
				{

					// Треугольник (оставляем без индексов для демонстрации)
				std::vector<CE::CEMeshComponent::Vertex> vertices1 = {
					{{0.0f, -0.3f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Красный треугольник
					{{0.3f, 0.3f, 0.0f}, {0.0f, 1.0f, 0.0f}},
					{{-0.3f, 0.3f, 0.0f}, {0.0f, 0.0f, 1.0f}}
					};
				MeshComponent->SetVertices ( vertices1 );

				}
			}

		void Tick ( float deltaTime ) override
			{
			CE::CEActor::Tick ( deltaTime );


			static float rotation = 0.0f;
			rotation += deltaTime * 45.0f;
			this->GetTransform ()->SetRotation ( CE::Math::Vector3 ( 0.0f, 0.0f, rotation ) );

			}

	private:
		CE::CEMeshComponent * MeshComponent = nullptr;
	};

class CTestActor2 : public CE::CEActor
	{
	public:
		CTestActor2 ( const std::string & name ) : CEActor ( name )
			{
			SetName ( name );

			// Создаем меш компонент через CreateComponent
			MeshComponent = CreateComponent<CE::CEMeshComponent> ();
			if (MeshComponent)
				{
				MeshComponent->SetName ( name + "_Mesh" );
				CE_DEBUG ( "Created mesh component for actor: {}", name );
				}
			else
				{
				CE_DEBUG ( "Failed to create mesh component for actor: {}", name );
				}
			}

		void BeginPlay () override
			{
			CE::CEActor::BeginPlay ();
			CE_DEBUG ( "CTestActor '{}' BeginPlay", GetName () );

			// Используем компонент трансформации для позиционирования
			auto * transform = GetTransform ();
			if (transform)
				{
				transform->SetPosition ( CE::Math::Vector3 ( -.5f, -.5f, -5.f ) );
				transform->SetScale ( 2.0f );
				}

				// Устанавливаем вершины и индексы
			if (MeshComponent)
				{


					// Квадрат с индексами (4 вершины, 6 индексов)
				std::vector<CE::CEMeshComponent::Vertex> vertices2 = {
					{{-0.2f, -0.2f, 0.0f}, {1.0f, 1.0f, 0.0f}},  // 0: Желтый
					{{0.2f, -0.2f, 0.0f}, {0.0f, 1.0f, 1.0f}},   // 1: Голубой  
					{{0.2f, 0.2f, 0.0f}, {1.0f, 0.0f, 1.0f}},    // 2: Пурпурный
					{{-0.2f, 0.2f, 0.0f}, {1.0f, 1.0f, 1.0f}}    // 3: Белый
					};

				std::vector<uint32_t> indices2 = {
					0, 1, 2,  // Первый треугольник
					2, 3, 0   // Второй треугольник
					};

				MeshComponent->SetVertices ( vertices2 );
				MeshComponent->SetIndices ( indices2 );
				CE_DEBUG ( "Set square vertices for TestActor2 - {} vertices, {} indices",
						   vertices2.size (), indices2.size () );

				}
			}

		void Tick ( float deltaTime ) override
			{
			CE::CEActor::Tick ( deltaTime );


			static float rotation = 0.0f;
			rotation -= deltaTime * 95.0f;
			this->GetTransform ()->SetRotation ( CE::Math::Vector3 ( rotation, -rotation, rotation ) );

			}

	private:
		CE::CEMeshComponent * MeshComponent = nullptr;
	};

void ChudEngineApp::Initialize ()
	{
	CE_CORE_DEBUG ( "ChudEngineApp::Initialize() started" );

	// Create window
	Window = std::make_unique<CE::CEWindow> ( "ChudEngine", 1280, 720 );
	CE_CORE_DEBUG ( "Window object created" );

	if (!Window->Initialize ())
		{
		CE_CORE_ERROR ( "Failed to initialize window" );
		return;
		}
	CE_CORE_DEBUG ( "Window initialized successfully" );

	// Create renderer
	Renderer = std::make_unique<CE::CEVulkanRenderer> ();
	CE_CORE_DEBUG ( "Renderer object created" );

	if (!Renderer->Initialize ( Window.get () ))
		{
		CE_CORE_ERROR ( "Failed to initialize Vulkan renderer" );
		return;
		}

		// Устанавливаем текущее приложение в рендерере
	static_cast< CE::CEVulkanRenderer * >( Renderer.get () )->SetCurrentApplication ( this );

	CE_CORE_DEBUG ( "Vulkan renderer initialized successfully" );

	// Настраиваем камеру
	SetupCamera ();

	// Create world
	World = std::make_unique<CE::CEWorld> ();
	World->SetName ( "MainWorld" );
	CE_CORE_DEBUG ( "World created" );

	CreateTestScene ();

	SetInitialized ( true );
	CE_CORE_DEBUG ( "ChudEngine Application initialized successfully" );
	}


void ChudEngineApp::Update ( float deltaTime )
	{

	static float rotation = 0.0f;
	rotation += deltaTime * 1.0f;
	auto * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );
	CameraPosition = CE::Math::Vector3 ( 0.0f, 0.0f, -2.0f ); // Камера смотрит с позиции (0,0,-2)
	CameraTarget = CE::Math::Vector3 ( 0.f, 0.0f, 0.0f );   // Смотрит в центр
	
	vulkanRenderer->SetCameraParameters ( CameraPosition, CameraTarget, CameraFOV );
	if (World)
		{
		World->Tick ( deltaTime );
		}
	}

void ChudEngineApp::Render ()
	{
	if (Renderer && World)
		{
			// Обновляем параметры камеры перед рендерингом
		auto * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );
		if (vulkanRenderer)
			{
			vulkanRenderer->SetCameraParameters ( CameraPosition, CameraTarget, CameraFOV );
			}

			// Используем RenderWorld для рендеринга
		vulkanRenderer->RenderWorld ( World.get () );
		}
	}

void ChudEngineApp::Shutdown ()
	{
	CE_CORE_DEBUG ( "Shutting down ChudEngine Application" );

	// Правильный порядок shutdown:
	if (World)
		{
		World->Destroy ();
		World.reset ();
		}

	if (Renderer)
		{
		Renderer->Shutdown ();
		Renderer.reset ();
		}

	if (Window)
		{
		Window->Shutdown ();
		Window.reset ();
		}

	CEApplication::Shutdown ();
	}

void ChudEngineApp::InitializeMeshComponents ()
	{
	if (!Renderer || !World)
		return;

	auto * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );

	// Получаем все меш компоненты и инициализируем их
	auto actors = World->GetActors ();
	for (auto * actor : actors)
		{
		if (actor)
			{
			auto meshComp = actor->GetComponent<CE::CEMeshComponent> ();
			if (meshComp)
				{
					// Проверяем, есть ли вершины
				auto vertices = meshComp->GetVertices ();
				if (vertices.empty ())
					{
					CE_CORE_WARN ( "Mesh component for actor '{}' has no vertices", actor->GetName () );
					continue;
					}

				CE_CORE_DEBUG ( "Initializing mesh component for actor '{}' with {} vertices, {} indices",
								actor->GetName (), vertices.size (), meshComp->GetIndices ().size () );

							// Инициализируем вершинный буфер через рендерер
				if (meshComp->CreateVertexBuffer ( vulkanRenderer ))
					{
					CE_CORE_DEBUG ( "Successfully created vertex buffer for actor: {}", actor->GetName () );
					}
				else
					{
					CE_CORE_ERROR ( "Failed to create vertex buffer for actor: {}", actor->GetName () );
					}

				// Инициализируем индексный буфер (если есть индексы)
				if (meshComp->HasIndices ())
					{
					if (meshComp->CreateIndexBuffer ( vulkanRenderer ))
						{
						CE_CORE_DEBUG ( "Successfully created index buffer for actor: {} with {} indices",
										actor->GetName (), meshComp->GetIndices ().size () );
						}
					else
						{
						CE_CORE_ERROR ( "Failed to create index buffer for actor: {}", actor->GetName () );
						}
					}
				}
			}
		}
	}

void ChudEngineApp::CreateTestScene ()
	{
	CE_CORE_DEBUG ( "Creating test scene" );

	// Create test actors
	auto * actor1 = new CTestActor ( "TestActor1" );
	auto * actor2 = new CTestActor2 ( "TestActor2" );

	actor1->GetTransform ()->SetPosition ( { 1.f,2.f,3.f } );
	actor2->GetTransform ()->SetPosition ( { 0.5f,2.f,2.f } );



	World->SpawnActor ( actor1 );
	World->SpawnActor ( actor2 );

	// Начинаем игру после спавна всех акторов
	World->BeginPlay ();

	// Инициализируем вершинные буферы после создания мира и BeginPlay
	InitializeMeshComponents ();

	CE_CORE_DEBUG ( "Test scene created with {} actors", World->GetActors ().size () );
	}