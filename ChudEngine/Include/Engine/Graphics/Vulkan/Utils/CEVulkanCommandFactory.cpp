#include "Graphics/Vulkan/Utils/CEVulkanCommandFactory.hpp"
#include "Graphics/Vulkan/Memory/CEVulkanBuffer.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    void CEVulkanCommandFactory::AddClearColorCommand ( VkCommandBuffer commandBuffer,
                                                        VkRenderPass renderPass,
                                                        VkFramebuffer framebuffer,
                                                        VkExtent2D extent,
                                                        const Math::Vector4 & clearColor )
        {
        std::array<VkClearValue, 1> clearValues = {};
        clearValues[ 0 ].color = { {clearColor.x, clearColor.y, clearColor.z, clearColor.w} };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size () );
        renderPassInfo.pClearValues = clearValues.data ();

        vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
        }

    void CEVulkanCommandFactory::AddClearDepthStencilCommand ( VkCommandBuffer commandBuffer,
                                                               VkRenderPass renderPass,
                                                               VkFramebuffer framebuffer,
                                                               VkExtent2D extent,
                                                               float depth,
                                                               uint32_t stencil )
        {
        std::array<VkClearValue, 1> clearValues = {};
        clearValues[ 0 ].depthStencil = { depth, stencil };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size () );
        renderPassInfo.pClearValues = clearValues.data ();

        vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
        }

    void CEVulkanCommandFactory::AddBufferUploadCommand ( VkCommandBuffer commandBuffer,
                                                          CEVulkanBuffer * dstBuffer,
                                                          const void * data,
                                                          VkDeviceSize size,
                                                          VkDeviceSize offset )
        {
        if (!dstBuffer || !data || size == 0)
            {
            CE_CORE_WARN ( "Invalid parameters for buffer upload" );
            return;
            }

            // In a real implementation, you'd use a staging buffer
            // For now, we assume the buffer is host-visible
        dstBuffer->UploadData ( data, size, offset );
        }

    void CEVulkanCommandFactory::AddBufferCopyCommand ( VkCommandBuffer commandBuffer,
                                                        CEVulkanBuffer * srcBuffer,
                                                        CEVulkanBuffer * dstBuffer,
                                                        VkDeviceSize size )
        {
        if (!srcBuffer || !dstBuffer || size == 0)
            {
            CE_CORE_WARN ( "Invalid parameters for buffer copy" );
            return;
            }

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer ( commandBuffer, srcBuffer->GetBuffer (), dstBuffer->GetBuffer (), 1, &copyRegion );
        }

    void CEVulkanCommandFactory::AddImageLayoutTransition ( VkCommandBuffer commandBuffer,
                                                            VkImage image,
                                                            VkFormat format,
                                                            VkImageLayout oldLayout,
                                                            VkImageLayout newLayout,
                                                            uint32_t mipLevels,
                                                            uint32_t layerCount )
        {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
        else
            {
            CE_CORE_WARN ( "Unsupported layout transition" );
            return;
            }

        vkCmdPipelineBarrier (
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        }

    void CEVulkanCommandFactory::AddMemoryBarrier ( VkCommandBuffer commandBuffer,
                                                    VkPipelineStageFlags srcStageMask,
                                                    VkPipelineStageFlags dstStageMask,
                                                    VkAccessFlags srcAccessMask,
                                                    VkAccessFlags dstAccessMask )
        {
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = srcAccessMask;
        memoryBarrier.dstAccessMask = dstAccessMask;

        vkCmdPipelineBarrier (
            commandBuffer,
            srcStageMask, dstStageMask,
            0,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr
        );
        }

    void CEVulkanCommandFactory::AddBeginRenderPassCommand ( VkCommandBuffer commandBuffer,
                                                             VkRenderPass renderPass,
                                                             VkFramebuffer framebuffer,
                                                             VkExtent2D extent,
                                                             const std::vector<VkClearValue> & clearValues )
        {
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size () );
        renderPassInfo.pClearValues = clearValues.data ();

        vkCmdBeginRenderPass ( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
        }

    void CEVulkanCommandFactory::AddEndRenderPassCommand ( VkCommandBuffer commandBuffer )
        {
        vkCmdEndRenderPass ( commandBuffer );
        }

    void CEVulkanCommandFactory::AddBindPipelineCommand ( VkCommandBuffer commandBuffer,
                                                          VkPipeline pipeline )
        {
        vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
        }

    void CEVulkanCommandFactory::AddBindVertexBuffersCommand ( VkCommandBuffer commandBuffer,
                                                               uint32_t firstBinding,
                                                               uint32_t bindingCount,
                                                               const VkBuffer * buffers,
                                                               const VkDeviceSize * offsets )
        {
        vkCmdBindVertexBuffers ( commandBuffer, firstBinding, bindingCount, buffers, offsets );
        }

    void CEVulkanCommandFactory::AddBindIndexBufferCommand ( VkCommandBuffer commandBuffer,
                                                             VkBuffer buffer,
                                                             VkDeviceSize offset,
                                                             VkIndexType indexType )
        {
        vkCmdBindIndexBuffer ( commandBuffer, buffer, offset, indexType );
        }

    void CEVulkanCommandFactory::AddDrawCommand ( VkCommandBuffer commandBuffer,
                                                  uint32_t vertexCount,
                                                  uint32_t instanceCount,
                                                  uint32_t firstVertex,
                                                  uint32_t firstInstance )
        {
        vkCmdDraw ( commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
        }

    void CEVulkanCommandFactory::AddDrawIndexedCommand ( VkCommandBuffer commandBuffer,
                                                         uint32_t indexCount,
                                                         uint32_t instanceCount,
                                                         uint32_t firstIndex,
                                                         int32_t vertexOffset,
                                                         uint32_t firstInstance )
        {
        vkCmdDrawIndexed ( commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
        }

    void CEVulkanCommandFactory::AddPushConstantsCommand ( VkCommandBuffer commandBuffer,
                                                           VkPipelineLayout pipelineLayout,
                                                           VkShaderStageFlags stageFlags,
                                                           uint32_t offset,
                                                           uint32_t size,
                                                           const void * data )
        {
        vkCmdPushConstants ( commandBuffer, pipelineLayout, stageFlags, offset, size, data );
        }
    }