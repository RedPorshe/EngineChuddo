// Graphics/Vulkan/Rendering/CEVulkanRenderPassManager.hpp
#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace CE
    {
    class CEVulkanContext;

    struct RenderPassInfo
        {
        std::string name;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
        };

    class CEVulkanRenderPassManager
        {
        public:
            CEVulkanRenderPassManager ( CEVulkanContext * context );
            ~CEVulkanRenderPassManager ();

            bool Initialize ();
            void Shutdown ();

            VkRenderPass CreateRenderPass ( const RenderPassInfo & info );
            VkRenderPass GetRenderPass ( const std::string & name ) const;
            void DestroyRenderPass ( const std::string & name );

            VkRenderPass GetMainRenderPass () const { return GetRenderPass ( "main" ); }
            VkRenderPass GetShadowRenderPass () const { return GetRenderPass ( "shadow" ); }
            VkRenderPass GetUiRenderPass () const { return GetRenderPass ( "ui" ); }
            VkRenderPass GetPostProcessRenderPass () const { return GetRenderPass ( "post_process" ); }

        private:
            CEVulkanContext * m_Context = nullptr;
            std::unordered_map<std::string, VkRenderPass> m_RenderPasses;
        };
    }