#include "Graphics/Vulkan/Descriptors/CEVulkanDescriptorLayoutCache.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    bool DescriptorLayoutInfo::operator==( const DescriptorLayoutInfo & other ) const
        {
        if (bindings.size () != other.bindings.size ())
            {
            return false;
            }

        for (size_t i = 0; i < bindings.size (); i++)
            {
            if (bindings[ i ].binding != other.bindings[ i ].binding ||
                 bindings[ i ].descriptorType != other.bindings[ i ].descriptorType ||
                 bindings[ i ].descriptorCount != other.bindings[ i ].descriptorCount ||
                 bindings[ i ].stageFlags != other.bindings[ i ].stageFlags)
                {
                return false;
                }
            }

        return true;
        }

    size_t DescriptorLayoutInfo::hash () const
        {
        size_t result = std::hash<size_t> ()( bindings.size () );

        for (const VkDescriptorSetLayoutBinding & binding : bindings)
            {
            size_t binding_hash = binding.binding | binding.descriptorType << 8 |
                binding.descriptorCount << 16 | binding.stageFlags << 24;
            result ^= std::hash<size_t> ()( binding_hash );
            }

        return result;
        }

    CEVulkanDescriptorLayoutCache::CEVulkanDescriptorLayoutCache ()
        : m_Context ( nullptr )
        {
        }

    CEVulkanDescriptorLayoutCache::~CEVulkanDescriptorLayoutCache ()
        {
        Shutdown ();
        }

    bool CEVulkanDescriptorLayoutCache::Initialize ( CEVulkanContext * context )
        {
        m_Context = context;
        CE_CORE_DEBUG ( "Descriptor layout cache initialized" );
        return true;
        }

    void CEVulkanDescriptorLayoutCache::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            for (auto & [info, layout] : m_LayoutCache)
                {
                if (layout != VK_NULL_HANDLE)
                    {
                    vkDestroyDescriptorSetLayout ( device, layout, nullptr );
                    }
                }
            m_LayoutCache.clear ();
            }

        CE_CORE_DEBUG ( "Descriptor layout cache shut down" );
        }

    VkDescriptorSetLayout CEVulkanDescriptorLayoutCache::CreateDescriptorLayout ( const DescriptorLayoutInfo & info )
        {
        auto it = m_LayoutCache.find ( info );
        if (it != m_LayoutCache.end ())
            {
            return it->second;
            }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast< uint32_t >( info.bindings.size () );
        layoutInfo.pBindings = info.bindings.data ();

        VkDescriptorSetLayout layout;
        VkResult result = vkCreateDescriptorSetLayout ( m_Context->GetDevice ()->GetDevice (), &layoutInfo, nullptr, &layout );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create descriptor set layout" );
            return VK_NULL_HANDLE;
            }

        m_LayoutCache[ info ] = layout;
        CE_CORE_DEBUG ( "Created descriptor set layout with {} bindings", info.bindings.size () );

        return layout;
        }

    void CEVulkanDescriptorLayoutCache::DestroyDescriptorLayout ( VkDescriptorSetLayout layout )
        {
        for (auto it = m_LayoutCache.begin (); it != m_LayoutCache.end (); ++it)
            {
            if (it->second == layout)
                {
                if (m_Context && m_Context->GetDevice ())
                    {
                    vkDestroyDescriptorSetLayout ( m_Context->GetDevice ()->GetDevice (), layout, nullptr );
                    }
                m_LayoutCache.erase ( it );
                break;
                }
            }
        }
    }