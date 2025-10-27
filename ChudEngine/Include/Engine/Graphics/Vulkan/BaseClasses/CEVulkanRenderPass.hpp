// Graphics/Vulkan/BaseClasses/CEVulkanRenderPass.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace CE
    {
    class CEVulkanContext;

    struct RenderPassAttachment
        {
        VkFormat format;
        VkSampleCountFlagBits samples;
        VkAttachmentLoadOp loadOp;
        VkAttachmentStoreOp storeOp;
        VkAttachmentLoadOp stencilLoadOp;
        VkAttachmentStoreOp stencilStoreOp;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
        };

    struct SubpassDescription
        {
        std::vector<VkAttachmentReference> colorAttachments;
        VkAttachmentReference depthAttachment;
        std::vector<VkAttachmentReference> inputAttachments;
        std::vector<uint32_t> preserveAttachments;
        };

    class CEVulkanRenderPass
        {
        public:
            CEVulkanRenderPass ( CEVulkanContext * context );
            ~CEVulkanRenderPass ();

            bool Create ( const std::vector<RenderPassAttachment> & attachments,
                          const std::vector<SubpassDescription> & subpasses,
                          const std::vector<VkSubpassDependency> & dependencies );
            void Destroy ();

            void Begin ( VkCommandBuffer commandBuffer, VkFramebuffer framebuffer,
                         VkExtent2D extent, const std::vector<VkClearValue> & clearValues );
            void End ( VkCommandBuffer commandBuffer );

            VkRenderPass GetHandle () const { return m_RenderPass; }

        private:
            CEVulkanContext * m_Context = nullptr;
            VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        };
    }