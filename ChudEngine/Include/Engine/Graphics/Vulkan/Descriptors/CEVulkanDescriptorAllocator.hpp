// Graphics/Vulkan/Descriptors/CEVulkanDescriptorAllocator.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

namespace CE
    {
    class CEVulkanContext;

    class CEVulkanDescriptorAllocator
        {
        public:
            struct PoolSizes
                {
                std::vector<std::pair<VkDescriptorType, float>> sizes = {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
                    };
                };

            CEVulkanDescriptorAllocator ();
            ~CEVulkanDescriptorAllocator ();

            bool Initialize ( CEVulkanContext * context );
            void Shutdown ();

            void ResetPools ();
            bool Allocate ( VkDescriptorSet * set, VkDescriptorSetLayout layout );

            VkDescriptorPool CreatePool ( const PoolSizes & poolSizes, int count, VkDescriptorPoolCreateFlags flags );
            void DestroyPool ( VkDescriptorPool pool );

        private:
            VkDescriptorPool GetCurrentPool ();
            VkDescriptorPool CreatePool ();

            CEVulkanContext * m_Context = nullptr;
            PoolSizes m_DescriptorSizes;
            std::vector<VkDescriptorPool> m_UsedPools;
            std::vector<VkDescriptorPool> m_FreePools;
            VkDescriptorPool m_CurrentPool = VK_NULL_HANDLE;
        };
    }