// Graphics/Vulkan/Materials/CEVulkanMaterialManager.hpp
#pragma once
#include <unordered_map>
#include <memory>
#include <string>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanPipelineManager;
    class CEVulkanTextureManager;
    class CEVulkanMaterial;

    class CEVulkanMaterialManager
        {
        public:
            CEVulkanMaterialManager ();
            ~CEVulkanMaterialManager ();

            bool Initialize ( CEVulkanContext * context,
                              CEVulkanPipelineManager * pipelineManager,
                              CEVulkanTextureManager * textureManager );
            void Shutdown ();

            std::shared_ptr<CEVulkanMaterial> CreateMaterial ( const std::string & name );
            std::shared_ptr<CEVulkanMaterial> LoadMaterial ( const std::string & filepath );
            std::shared_ptr<CEVulkanMaterial> GetMaterial ( const std::string & name );
            void DestroyMaterial ( const std::string & name );

            void ReloadAllMaterials ();
            void UpdateMaterials ( uint32_t currentFrame );

            std::shared_ptr<CEVulkanMaterial> GetDefaultMaterial ();
            std::shared_ptr<CEVulkanMaterial> GetErrorMaterial ();

        private:
            bool CreateDefaultMaterials ();

            CEVulkanContext * m_Context = nullptr;
            CEVulkanPipelineManager * m_PipelineManager = nullptr;
            CEVulkanTextureManager * m_TextureManager = nullptr;

            std::unordered_map<std::string, std::shared_ptr<CEVulkanMaterial>> m_Materials;
            std::shared_ptr<CEVulkanMaterial> m_DefaultMaterial;
            std::shared_ptr<CEVulkanMaterial> m_ErrorMaterial;
        };
    }