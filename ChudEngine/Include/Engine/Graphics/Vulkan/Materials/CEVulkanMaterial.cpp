#include "Graphics/Vulkan/Materials/CEVulkanMaterial.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanPipelineManager.hpp"
#include "Graphics/Vulkan/Managers/CEVulkanTextureManager.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanMaterial::CEVulkanMaterial ()
        : m_Context ( nullptr )
        , m_PipelineManager ( nullptr )
        , m_TextureManager ( nullptr )
        , m_PipelineType ( PipelineType::StaticMesh )
        , m_IsTransparent ( false )
        , m_Initialized ( false )
        {
        }

    CEVulkanMaterial::~CEVulkanMaterial ()
        {
        Shutdown ();
        }

    bool CEVulkanMaterial::Initialize ( CEVulkanContext * context,
                                        CEVulkanPipelineManager * pipelineManager,
                                        CEVulkanTextureManager * textureManager,
                                        const std::string & name )
        {
        if (m_Initialized)
            {
            CE_CORE_WARN ( "Material already initialized: {}", name );
            return true;
            }

        m_Context = context;
        m_PipelineManager = pipelineManager;
        m_TextureManager = textureManager;
        m_Name = name;

        try
            {
            if (!CreateDescriptorSets ())
                {
                throw std::runtime_error ( "Failed to create descriptor sets" );
                }

            m_Initialized = true;
            CE_CORE_DEBUG ( "Material initialized: {}", name );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize material {}: {}", name, e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanMaterial::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
// Descriptor sets are managed by the allocator, so we don't destroy them here
            }

        m_Parameters.clear ();
        m_DescriptorSets.clear ();
        m_Initialized = false;

        CE_CORE_DEBUG ( "Material shut down: {}", m_Name );
        }

    void CEVulkanMaterial::SetParameter ( const std::string & name, const MaterialParameter & parameter )
        {
        m_Parameters[ name ] = parameter;

        // Mark material as dirty for descriptor set update
        // In a full implementation, we'd track dirty state and update descriptors only when needed
        }

    MaterialParameter CEVulkanMaterial::GetParameter ( const std::string & name ) const
        {
        auto it = m_Parameters.find ( name );
        if (it != m_Parameters.end ())
            {
            return it->second;
            }

            // Return default parameter based on type
        MaterialParameter param;
        // This would need type information to return proper defaults
        return param;
        }

    bool CEVulkanMaterial::HasParameter ( const std::string & name ) const
        {
        return m_Parameters.find ( name ) != m_Parameters.end ();
        }

    void CEVulkanMaterial::Bind ( VkCommandBuffer commandBuffer )
        {
        if (!m_Initialized || !m_PipelineManager) return;

        auto pipeline = m_PipelineManager->GetPipeline ( m_PipelineType );
        if (pipeline)
            {
            pipeline->Bind ( commandBuffer );

            // Bind descriptor sets
            if (!m_DescriptorSets.empty ())
                {
                vkCmdBindDescriptorSets (
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline->GetLayout (),
                    0,
                    static_cast< uint32_t >( m_DescriptorSets.size () ),
                    m_DescriptorSets.data (),
                    0, nullptr
                );
                }
            }
        }

    void CEVulkanMaterial::UpdateUniforms ( uint32_t currentFrame )
        {
            // This would update uniform buffers with current material parameters
            // Implementation depends on your buffer management strategy

        for (const auto & [name, param] : m_Parameters)
            {
            switch (param.type)
                {
                    case MaterialParameter::Type::Float:
                        // Update float uniform
                        break;
                    case MaterialParameter::Type::Vector3:
                        // Update vector3 uniform
                        break;
                    case MaterialParameter::Type::Texture:
                        // Update texture descriptor
                        break;
                    // ... other types
                    default:
                        break;
                }
            }
        }

    bool CEVulkanMaterial::CreateDescriptorSets ()
        {
            // This would create descriptor sets for the material
            // For now, we'll create empty descriptor sets

        if (!m_PipelineManager) return false;

        auto pipeline = m_PipelineManager->GetPipeline ( m_PipelineType );
        if (!pipeline) return false;

        // In a full implementation, we'd use the descriptor allocator
        // to allocate descriptor sets for this material

        CE_CORE_DEBUG ( "Created descriptor sets for material: {}", m_Name );
        return true;
        }

    void CEVulkanMaterial::UpdateDescriptorSets ()
        {
            // This would update descriptor sets with current material parameters
            // Implementation would write to descriptor sets with current textures and buffers
        }
    }