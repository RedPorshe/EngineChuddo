#include "ChudEngineApp.hpp"
#include "Core/CEObject/CEActor.hpp"
#include "Graphics/Vulkan/CEVulkanRenderer.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Core/CEObject/CEWorld.hpp"
#include "Core/CEObject/Components/CEMeshComponent.hpp"
#include "../MatrixDebugUtils.hpp"
#include "Core/Logger.h"

void ChudEngineApp::SetupCamera ()
    {
    CameraPosition = CE::Math::Vector3 ( 0.0f, 0.0f, 2.0f );
    CameraTarget = CE::Math::Vector3 ( 0.0f, 0.0f, 0.0f );
    CameraFOV = 45.0f;

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

            // ПРИБЛИЖАЕМ К КАМЕРЕ
            auto * transform = GetTransform ();
            if (transform)
                {
                transform->SetPosition ( CE::Math::Vector3 ( 2.0f, 0.0f, -5.0f ) ); // Ближе!
                transform->SetScale ( 1.0f );
                }

                // УВЕЛИЧИВАЕМ ВЕРШИНЫ ТРЕУГОЛЬНИКА
            if (MeshComponent)
                {
                std::vector<CE::CEMeshComponent::Vertex> vertices = {
                    {{0.0f, -3.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Красный - УВЕЛИЧИЛИ
                    {{3.0f, 3.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},   // Зеленый - УВЕЛИЧИЛИ
                    {{-3.0f, 3.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Синий - УВЕЛИЧИЛИ
                    };

                MeshComponent->SetVertices ( vertices );
                CE_DEBUG ( "Set triangle vertices for {} - {} vertices",
                           GetName (), vertices.size () );
                }
            }

        void Tick ( float deltaTime ) override
            {
            CE::CEActor::Tick ( deltaTime );

            static float rotation = 0.0f;
            rotation += deltaTime * 45.0f;
            GetTransform ()->SetRotation ( CE::Math::Vector3 ( 0.0f, 0.0f, rotation ) );
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
            CE_DEBUG ( "CTestActor2 '{}' BeginPlay", GetName () );

            // ПРИБЛИЖАЕМ К КАМЕРЕ
            auto * transform = GetTransform ();
            if (transform)
                {
                transform->SetPosition ( CE::Math::Vector3 ( -2.0f, 0.0f, -5.0f ) ); // Ближе!
                transform->SetScale ( 1.0f );
                }

                // УВЕЛИЧИВАЕМ ВЕРШИНЫ КВАДРАТА
            if (MeshComponent)
                {
                std::vector<CE::CEMeshComponent::Vertex> vertices = {
                    {{-2.0f, -2.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},  // Желтый - УВЕЛИЧИЛИ
                    {{2.0f, -2.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},   // Голубой - УВЕЛИЧИЛИ
                    {{2.0f, 2.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},    // Пурпурный - УВЕЛИЧИЛИ
                    {{-2.0f, 2.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}    // Белый - УВЕЛИЧИЛИ
                    };

                std::vector<uint32_t> indices = {
                    0, 1, 2,  // Первый треугольник
                    2, 3, 0   // Второй треугольник
                    };

                MeshComponent->SetVertices ( vertices );
                MeshComponent->SetIndices ( indices );
                CE_DEBUG ( "Set square vertices for {} - {} vertices, {} indices",
                           GetName (), vertices.size (), indices.size () );
                }
            }

        void Tick ( float deltaTime ) override
            {
            CE::CEActor::Tick ( deltaTime );

            static float rotation = 0.0f;
            rotation -= deltaTime * 95.0f;
            GetTransform ()->SetRotation ( CE::Math::Vector3 ( rotation, -rotation, rotation ) );
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
   

    auto * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );
  //  CameraPosition = CE::Math::Vector3 ( 0.0f, 0.0f, 5.0f );
   // CameraTarget = CE::Math::Vector3 ( 0.0f, 0.0f, 0.0f );

   // vulkanRenderer->SetCameraParameters ( CameraPosition, CameraTarget, CameraFOV );

    static float rotation = 0.0f;
    rotation -= deltaTime * 95.0f;
    //GetTransform ()->SetRotation ( CE::Math::Vector3 ( rotation, -rotation, rotation ) );

    if (World)
        {
        auto& actors = World->GetActors ();
        for (auto at : actors)
            {
          
         
            }
        World->Tick ( deltaTime );
        }
    }

void ChudEngineApp::Render ()
    {
    if (Renderer && World)
        {
        auto * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );
        if (vulkanRenderer)
            {
            vulkanRenderer->SetCameraParameters ( CameraPosition, CameraTarget, CameraFOV );
            }

        vulkanRenderer->RenderWorld ( World.get () );
        }
    }

void ChudEngineApp::Shutdown ()
    {
    CE_CORE_DEBUG ( "Shutting down ChudEngine Application" );

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

    auto actors = World->GetActors ();
    for (auto * actor : actors)
        {
        if (actor)
            {
            auto meshComp = actor->GetComponent<CE::CEMeshComponent> ();
            if (meshComp)
                {
                auto vertices = meshComp->GetVertices ();
                if (vertices.empty ())
                    {
                    CE_CORE_WARN ( "Mesh component for actor '{}' has no vertices", actor->GetName () );
                    continue;
                    }

                CE_CORE_DEBUG ( "Initializing mesh component for actor '{}' with {} vertices, {} indices",
                                actor->GetName (), vertices.size (), meshComp->GetIndices ().size () );

                            // Создаем вершинный буфер
                if (meshComp->CreateVertexBuffer ( vulkanRenderer ))
                    {
                    CE_CORE_DEBUG ( "Successfully created vertex buffer for actor: {}", actor->GetName () );
                    }
                else
                    {
                    CE_CORE_ERROR ( "Failed to create vertex buffer for actor: {}", actor->GetName () );
                    }

                    // Создаем индексный буфер (если есть индексы)
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
    // ВРЕМЕННО: Простейший треугольник в центре
    auto * testActor = new CE::CEActor ( "SimpleTest" );
    auto * meshComponent = testActor->CreateComponent<CE::CEMeshComponent> ();

    // Простой треугольник с яркими цветами
    std::vector< CE::CEMeshComponent::Vertex > vertices = {
      { {0.0f, -0.5f, 0.0f} , { 1.0f, 0.0f, 0.0f} },  // нижняя вершина
      { {0.5f, 0.5f, 0.0f} , {0.0f, 1.0f, 0.0f} },   // правая вершина  
      { {  -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }   // левая вершина
        };

    meshComponent->SetVertices ( vertices );
    testActor->GetTransform ()->SetPosition ( CE::Math::Vector3 ( 0.0f, 0.0f, -3.0f ) );

    World->SpawnActor ( testActor );
    CE_DEBUG ( "Simple test scene created with 1 triangle" );
    InitializeMeshComponents ();
    }
