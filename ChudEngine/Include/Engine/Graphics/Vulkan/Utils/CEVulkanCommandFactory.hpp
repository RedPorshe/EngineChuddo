// Graphics/Vulkan/Utils/CEVulkanCommandFactory.hpp
#pragma once
#include <vulkan/vulkan.h>
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

namespace CE
    {
    class CEVulkanBuffer;

    class CEVulkanCommandFactory
        {
        public:
            static void AddClearColorCommand ( VkCommandBuffer commandBuffer,
                                               VkRenderPass renderPass,
                                               VkFramebuffer framebuffer,
                                               VkExtent2D extent,
                                               const Math::Vector4 & clearColor = { 0.0f, 0.0f, 0.0f, 1.0f } );

            static void AddClearDepthStencilCommand ( VkCommandBuffer commandBuffer,
                                                      VkRenderPass renderPass,
                                                      VkFramebuffer framebuffer,
                                                      VkExtent2D extent,
                                                      float depth = 1.0f,
                                                      uint32_t stencil = 0 );

            static void AddBufferUploadCommand ( VkCommandBuffer commandBuffer,
                                                 CEVulkanBuffer * dstBuffer,
                                                 const void * data,
                                                 VkDeviceSize size,
                                                 VkDeviceSize offset = 0 );

            static void AddBufferCopyCommand ( VkCommandBuffer commandBuffer,
                                               CEVulkanBuffer * srcBuffer,
                                               CEVulkanBuffer * dstBuffer,
                                               VkDeviceSize size );

            static void AddImageLayoutTransition ( VkCommandBuffer commandBuffer,
                                                   VkImage image,
                                                   VkFormat format,
                                                   VkImageLayout oldLayout,
                                                   VkImageLayout newLayout,
                                                   uint32_t mipLevels = 1,
                                                   uint32_t layerCount = 1 );

            static void AddMemoryBarrier ( VkCommandBuffer commandBuffer,
                                           VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask,
                                           VkAccessFlags srcAccessMask,
                                           VkAccessFlags dstAccessMask );

            static void AddBeginRenderPassCommand ( VkCommandBuffer commandBuffer,
                                                    VkRenderPass renderPass,
                                                    VkFramebuffer framebuffer,
                                                    VkExtent2D extent,
                                                    const std::vector<VkClearValue> & clearValues );

            static void AddEndRenderPassCommand ( VkCommandBuffer commandBuffer );

            static void AddBindPipelineCommand ( VkCommandBuffer commandBuffer,
                                                 VkPipeline pipeline );

            static void AddBindVertexBuffersCommand ( VkCommandBuffer commandBuffer,
                                                      uint32_t firstBinding,
                                                      uint32_t bindingCount,
                                                      const VkBuffer * buffers,
                                                      const VkDeviceSize * offsets );

            static void AddBindIndexBufferCommand ( VkCommandBuffer commandBuffer,
                                                    VkBuffer buffer,
                                                    VkDeviceSize offset,
                                                    VkIndexType indexType );

            static void AddDrawCommand ( VkCommandBuffer commandBuffer,
                                         uint32_t vertexCount,
                                         uint32_t instanceCount = 1,
                                         uint32_t firstVertex = 0,
                                         uint32_t firstInstance = 0 );

            static void AddDrawIndexedCommand ( VkCommandBuffer commandBuffer,
                                                uint32_t indexCount,
                                                uint32_t instanceCount = 1,
                                                uint32_t firstIndex = 0,
                                                int32_t vertexOffset = 0,
                                                uint32_t firstInstance = 0 );

            static void AddPushConstantsCommand ( VkCommandBuffer commandBuffer,
                                                  VkPipelineLayout pipelineLayout,
                                                  VkShaderStageFlags stageFlags,
                                                  uint32_t offset,
                                                  uint32_t size,
                                                  const void * data );
        };
    }