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
    CameraPosition = CE::Math::Vector3 ( 0.0f, 0.0f, 10.0f );
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

            // Устанавливаем позицию и масштаб
            auto * transform = GetTransform ();
            if (transform)
                {
                transform->SetPosition ( CE::Math::Vector3 ( 0.5f, 0.5f, 0.0f ) );
                transform->SetScale ( 1.0f );
                }

                // Устанавливаем вершины треугольника
            if (MeshComponent)
                {
                std::vector<CE::CEMeshComponent::Vertex> vertices = {
                    {{0.0f, -0.3f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Красный
                    {{0.3f, 0.3f, 0.0f}, {0.0f, 1.0f, 0.0f}},   // Зеленый
                    {{-0.3f, 0.3f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Синий
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
          //  GetTransform ()->SetRotation ( CE::Math::Vector3 ( 0.0f, 0.0f, rotation ) );
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

            // Устанавливаем позицию и масштаб
            auto * transform = GetTransform ();
            if (transform)
                {
                transform->SetPosition ( CE::Math::Vector3 ( -0.5f, -0.5f, 0.0f ) );
                transform->SetScale ( 1.0f );
                }

                // Устанавливаем вершины и индексы квадрата
            if (MeshComponent)
                {
                std::vector<CE::CEMeshComponent::Vertex> vertices = {
                    {{-0.2f, -0.2f, 0.0f}, {1.0f, 1.0f, 0.0f}},  // Желтый
                    {{0.2f, -0.2f, 0.0f}, {0.0f, 1.0f, 1.0f}},   // Голубой  
                    {{0.2f, 0.2f, 0.0f}, {1.0f, 0.0f, 1.0f}},    // Пурпурный
                    {{-0.2f, 0.2f, 0.0f}, {1.0f, 1.0f, 1.0f}}    // Белый
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
          //  GetTransform ()->SetRotation ( CE::Math::Vector3 ( rotation, -rotation, rotation ) );
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
   
    CE::CEVulkanRenderer * vulkanRenderer = static_cast< CE::CEVulkanRenderer * >( Renderer.get () );

    // Создаем основной прямоугольник
    auto * mainActor = new CE::CEActor ( "TestMeshActor" );
    auto * mainMeshComponent = mainActor->CreateComponent<CE::CEMeshComponent> ();

    std::vector<CE::CEMeshComponent::Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // красный
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // зеленый
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},   // синий
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}}   // желтый
        };

    std::vector<uint32_t> indices = {
        0, 1, 2,  // первый треугольник
        2, 3, 0   // второй треугольник
        };

    mainMeshComponent->SetVertices ( vertices );
    mainMeshComponent->SetIndices ( indices );

    // Устанавливаем позицию для основного прямоугольника
    mainActor->GetTransform ()->SetPosition ( CE::Math::Vector3 ( 0.0f, 0.0f, 0.0f ) );

    // Создаем дополнительные акторы
    auto * testActor1 = new CTestActor ( "TestTriangle" );
    auto * testActor2 = new CTestActor2 ( "TestSquare" );
    
    mainActor->GetTransform ()->SetPosition ( CE::Math::Vector3 ( 0.0f, 0.0f, 0.0f ) );
    testActor1->GetTransform ()->SetPosition ( CE::Math::Vector3 ( 1.0f, 0.0f, 0.0f ) );  // Сдвиг по X
    testActor2->GetTransform ()->SetPosition ( CE::Math::Vector3 ( 0.0f, 1.0f, 0.0f ) );  // Сдвиг по Y
    
    // Добавляем ВСЕХ акторов в мир
    World->SpawnActor ( mainActor );
    World->SpawnActor ( testActor1 );
    World->SpawnActor ( testActor2 );
    

    CE_DEBUG ( "Test scene created with 3 actors" );

    // Инициализируем все меш компоненты
    InitializeMeshComponents ();
  
    }
