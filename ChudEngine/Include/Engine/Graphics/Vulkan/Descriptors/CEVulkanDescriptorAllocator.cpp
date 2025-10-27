#include "Graphics/Vulkan/Descriptors/CEVulkanDescriptorAllocator.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanDescriptorAllocator::CEVulkanDescriptorAllocator ()
        : m_Context ( nullptr )
        , m_CurrentPool ( VK_NULL_HANDLE )
        {
        }

    CEVulkanDescriptorAllocator::~CEVulkanDescriptorAllocator ()
        {
        Shutdown ();
        }

    bool CEVulkanDescriptorAllocator::Initialize ( CEVulkanContext * context )
        {
        m_Context = context;
        CE_CORE_DEBUG ( "Descriptor allocator initialized" );
        return true;
        }

    void CEVulkanDescriptorAllocator::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            for (auto pool : m_UsedPools)
                {
                vkDestroyDescriptorPool ( device, pool, nullptr );
                }
            for (auto pool : m_FreePools)
                {
                vkDestroyDescriptorPool ( device, pool, nullptr );
                }

            m_UsedPools.clear ();
            m_FreePools.clear ();
            m_CurrentPool = VK_NULL_HANDLE;
            }

        CE_CORE_DEBUG ( "Descriptor allocator shut down" );
        }

    void CEVulkanDescriptorAllocator::ResetPools ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            for (auto pool : m_UsedPools)
                {
                vkResetDescriptorPool ( device, pool, 0 );
                m_FreePools.push_back ( pool );
                }

            m_UsedPools.clear ();
            m_CurrentPool = VK_NULL_HANDLE;
            }
        }

    bool CEVulkanDescriptorAllocator::Allocate ( VkDescriptorSet * set, VkDescriptorSetLayout layout )
        {
        if (m_CurrentPool == VK_NULL_HANDLE)
            {
            m_CurrentPool = GetCurrentPool ();
            if (m_CurrentPool == VK_NULL_HANDLE)
                {
                return false;
                }
            m_UsedPools.push_back ( m_CurrentPool );
            }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.pSetLayouts = &layout;
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = m_CurrentPool;

        VkResult result = vkAllocateDescriptorSets ( m_Context->GetDevice ()->GetDevice (), &allocInfo, set );

        switch (result)
            {
                case VK_SUCCESS:
                    return true;

                case VK_ERROR_FRAGMENTED_POOL:
                case VK_ERROR_OUT_OF_POOL_MEMORY:
                    // Allocate new pool and retry
                    m_CurrentPool = GetCurrentPool ();
                    if (m_CurrentPool == VK_NULL_HANDLE)
                        {
                        return false;
                        }
                    m_UsedPools.push_back ( m_CurrentPool );

                    allocInfo.descriptorPool = m_CurrentPool;
                    result = vkAllocateDescriptorSets ( m_Context->GetDevice ()->GetDevice (), &allocInfo, set );
                    if (result == VK_SUCCESS)
                        {
                        return true;
                        }
                    break;

                default:
                    break;
            }

        CE_CORE_ERROR ( "Failed to allocate descriptor set" );
        return false;
        }

    VkDescriptorPool CEVulkanDescriptorAllocator::CreatePool ( const PoolSizes & poolSizes, int count, VkDescriptorPoolCreateFlags flags )
        {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve ( poolSizes.sizes.size () );

        for (auto sz : poolSizes.sizes)
            {
            sizes.push_back ( { sz.first, uint32_t ( sz.second * count ) } );
            }

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = flags;
        pool_info.maxSets = count;
        pool_info.poolSizeCount = ( uint32_t ) sizes.size ();
        pool_info.pPoolSizes = sizes.data ();

        VkDescriptorPool descriptorPool;
        VkResult result = vkCreateDescriptorPool ( m_Context->GetDevice ()->GetDevice (), &pool_info, nullptr, &descriptorPool );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create descriptor pool" );
            return VK_NULL_HANDLE;
            }

        return descriptorPool;
        }

    void CEVulkanDescriptorAllocator::DestroyPool ( VkDescriptorPool pool )
        {
        if (m_Context && m_Context->GetDevice ())
            {
            vkDestroyDescriptorPool ( m_Context->GetDevice ()->GetDevice (), pool, nullptr );
            }
        }

    VkDescriptorPool CEVulkanDescriptorAllocator::GetCurrentPool ()
        {
        if (!m_FreePools.empty ())
            {
            VkDescriptorPool pool = m_FreePools.back ();
            m_FreePools.pop_back ();
            return pool;
            }
        else
            {
            return CreatePool ( m_DescriptorSizes, 1000, 0 );
            }
        }

    VkDescriptorPool CEVulkanDescriptorAllocator::CreatePool ()
        {
        return CreatePool ( m_DescriptorSizes, 1000, 0 );
        }
    }