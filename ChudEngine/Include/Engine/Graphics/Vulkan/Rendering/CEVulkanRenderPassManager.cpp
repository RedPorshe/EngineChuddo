#include "Graphics/Vulkan/Rendering/CEVulkanRenderPassManager.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanRenderPassManager::CEVulkanRenderPassManager ( CEVulkanContext * context )
        : m_Context ( context )
        {
        }

    CEVulkanRenderPassManager::~CEVulkanRenderPassManager ()
        {
        Shutdown ();
        }

    bool CEVulkanRenderPassManager::Initialize ()
        {
        try
            {
           // Create main render pass (color + depth)
            RenderPassInfo mainPassInfo;
            mainPassInfo.name = "main";

            // Color attachment
            VkAttachmentDescription colorAttachment = {};
            colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB; // Typical swapchain format
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            mainPassInfo.attachments.push_back ( colorAttachment );

            // Depth attachment
            VkAttachmentDescription depthAttachment = {};
            depthAttachment.format = m_Context->GetDevice ()->FindDepthFormat ();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            mainPassInfo.attachments.push_back ( depthAttachment );

            // Subpass
            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkAttachmentReference depthAttachmentRef = {};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            mainPassInfo.subpasses.push_back ( subpass );

            // Dependency
            VkSubpassDependency dependency = {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            mainPassInfo.dependencies.push_back ( dependency );

            CreateRenderPass ( mainPassInfo );

            CE_CORE_DEBUG ( "Render pass manager initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize render pass manager: {}", e.what () );
                return false;
                }
        }

    void CEVulkanRenderPassManager::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            for (auto & [name, renderPass] : m_RenderPasses)
                {
                if (renderPass != VK_NULL_HANDLE)
                    {
                    vkDestroyRenderPass ( device, renderPass, nullptr );
                    }
                }
            m_RenderPasses.clear ();
            }

        CE_CORE_DEBUG ( "Render pass manager shut down" );
        }

    VkRenderPass CEVulkanRenderPassManager::CreateRenderPass ( const RenderPassInfo & info )
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for render pass creation" );
            }

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast< uint32_t >( info.attachments.size () );
        renderPassInfo.pAttachments = info.attachments.data ();
        renderPassInfo.subpassCount = static_cast< uint32_t >( info.subpasses.size () );
        renderPassInfo.pSubpasses = info.subpasses.data ();
        renderPassInfo.dependencyCount = static_cast< uint32_t >( info.dependencies.size () );
        renderPassInfo.pDependencies = info.dependencies.data ();

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkResult result = vkCreateRenderPass ( m_Context->GetDevice ()->GetDevice (), &renderPassInfo, nullptr, &renderPass );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create render pass: " + info.name );
            }

        m_RenderPasses[ info.name ] = renderPass;
        CE_CORE_DEBUG ( "Created render pass: {}", info.name );

        return renderPass;
        }

    VkRenderPass CEVulkanRenderPassManager::GetRenderPass ( const std::string & name ) const
        {
        auto it = m_RenderPasses.find ( name );
        if (it != m_RenderPasses.end ())
            {
            return it->second;
            }

        CE_CORE_WARN ( "Render pass not found: {}", name );
        return VK_NULL_HANDLE;
        }

    void CEVulkanRenderPassManager::DestroyRenderPass ( const std::string & name )
        {
        auto it = m_RenderPasses.find ( name );
        if (it != m_RenderPasses.end ())
            {
            if (m_Context && m_Context->GetDevice ())
                {
                vkDestroyRenderPass ( m_Context->GetDevice ()->GetDevice (), it->second, nullptr );
                }
            m_RenderPasses.erase ( it );
            CE_CORE_DEBUG ( "Destroyed render pass: {}", name );
            }
        }
    }