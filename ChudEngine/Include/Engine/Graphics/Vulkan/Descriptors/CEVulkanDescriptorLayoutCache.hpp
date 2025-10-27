// Graphics/Vulkan/Descriptors/CEVulkanDescriptorLayoutCache.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>

namespace CE
    {
    class CEVulkanContext;

    struct DescriptorLayoutInfo
        {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bool operator==( const DescriptorLayoutInfo & other ) const;
        size_t hash () const;
        };

    class CEVulkanDescriptorLayoutCache
        {
        public:
            CEVulkanDescriptorLayoutCache ();
            ~CEVulkanDescriptorLayoutCache ();

            bool Initialize ( CEVulkanContext * context );
            void Shutdown ();

            VkDescriptorSetLayout CreateDescriptorLayout ( const DescriptorLayoutInfo & info );
            void DestroyDescriptorLayout ( VkDescriptorSetLayout layout );

        private:
            struct DescriptorLayoutHash
                {
                std::size_t operator()( const DescriptorLayoutInfo & k ) const {
                    return k.hash ();
                    }
                };

            CEVulkanContext * m_Context = nullptr;
            std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_LayoutCache;
        };
    }