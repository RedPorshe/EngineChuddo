// Graphics/Vulkan/Memory/CEVulkanMemoryAllocator.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>

namespace CE
    {
    class CEVulkanContext;

    struct MemoryBlock
        {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize offset = 0;
        VkDeviceSize used = 0;
        uint32_t memoryType = 0;
        bool isMapped = false;
        void * mappedData = nullptr;
        };

    class CEVulkanMemoryAllocator
        {
        public:
            CEVulkanMemoryAllocator ();
            ~CEVulkanMemoryAllocator ();

            bool Initialize ( CEVulkanContext * context );
            void Shutdown ();

            std::shared_ptr<MemoryBlock> AllocateBuffer ( VkDeviceSize size, VkBufferUsageFlags usage,
                                                          VkMemoryPropertyFlags properties, VkBuffer & outBuffer );
            std::shared_ptr<MemoryBlock> AllocateImage ( VkDeviceSize size, VkImageUsageFlags usage,
                                                         VkMemoryPropertyFlags properties, VkImage & outImage );

            void FreeBuffer ( VkBuffer buffer );
            void FreeImage ( VkImage image );

            void Defragment ();
            void PrintStatistics () const;

        private:
            std::shared_ptr<MemoryBlock> FindSuitableBlock ( VkDeviceSize size, uint32_t memoryType );
            std::shared_ptr<MemoryBlock> CreateNewBlock ( VkDeviceSize size, uint32_t memoryType );

            CEVulkanContext * m_Context = nullptr;
            std::vector<std::shared_ptr<MemoryBlock>> m_MemoryBlocks;
            std::unordered_map<VkBuffer, std::shared_ptr<MemoryBlock>> m_BufferAllocations;
            std::unordered_map<VkImage, std::shared_ptr<MemoryBlock>> m_ImageAllocations;

            VkDeviceSize m_TotalAllocated = 0;
            VkDeviceSize m_TotalUsed = 0;
        };
    }