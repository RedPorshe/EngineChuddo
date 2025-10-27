#include "Graphics/Vulkan/Core/CEVulkanCommandBuffer.hpp"
#include "Graphics/Vulkan/Core/CEVulkanContext.hpp"
#include "Utils/Logger.hpp"

namespace CE
    {
    CEVulkanCommandBuffer::CEVulkanCommandBuffer ()
        : m_Context ( nullptr )
        , m_CommandPool ( VK_NULL_HANDLE )
        , m_CurrentFrame ( 0 )
        {
        }

    CEVulkanCommandBuffer::~CEVulkanCommandBuffer ()
        {
        Shutdown ();
        }

    bool CEVulkanCommandBuffer::Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight )
        {
        if (m_CommandPool != VK_NULL_HANDLE)
            {
            CE_CORE_WARN ( "Command buffer already initialized" );
            return true;
            }

        m_Context = context;

        try
            {
            CreateCommandPool ();
            CreateCommandBuffers ();

            CE_CORE_DEBUG ( "Command buffers initialized successfully" );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize command buffers: {}", e.what () );
                Shutdown ();
                return false;
                }
        }

    void CEVulkanCommandBuffer::Shutdown ()
        {
        if (m_Context && m_Context->GetDevice ())
            {
            VkDevice device = m_Context->GetDevice ()->GetDevice ();

            if (!m_CommandBuffers.empty ())
                {
                vkFreeCommandBuffers ( device, m_CommandPool,
                                       static_cast< uint32_t >( m_CommandBuffers.size () ),
                                       m_CommandBuffers.data () );
                m_CommandBuffers.clear ();
                }

            if (m_CommandPool != VK_NULL_HANDLE)
                {
                vkDestroyCommandPool ( device, m_CommandPool, nullptr );
                m_CommandPool = VK_NULL_HANDLE;
                }
            }

        m_CurrentFrame = 0;
        m_Context = nullptr;
        }

    void CEVulkanCommandBuffer::ResetCurrent ()
        {
        if (m_Context && m_Context->GetDevice () && !m_CommandBuffers.empty ())
            {
            VkCommandBuffer commandBuffer = m_CommandBuffers[ m_CurrentFrame ];
            vkResetCommandBuffer ( commandBuffer, 0 );
            }
        }

    void CEVulkanCommandBuffer::BeginRecording ()
        {
        if (m_Context && m_Context->GetDevice () && !m_CommandBuffers.empty ())
            {
            VkCommandBuffer commandBuffer = m_CommandBuffers[ m_CurrentFrame ];

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo = nullptr;

            VkResult result = vkBeginCommandBuffer ( commandBuffer, &beginInfo );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to begin recording command buffer" );
                }
            }
        }

    void CEVulkanCommandBuffer::EndRecording ()
        {
        if (m_Context && m_Context->GetDevice () && !m_CommandBuffers.empty ())
            {
            VkCommandBuffer commandBuffer = m_CommandBuffers[ m_CurrentFrame ];

            VkResult result = vkEndCommandBuffer ( commandBuffer );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to end recording command buffer" );
                }
            }
        }

    void CEVulkanCommandBuffer::Submit ( VkQueue queue, VkSemaphore waitSemaphore,
                                         VkSemaphore signalSemaphore, VkFence fence )
        {
        if (m_Context && m_Context->GetDevice () && !m_CommandBuffers.empty ())
            {
            VkCommandBuffer commandBuffer = m_CommandBuffers[ m_CurrentFrame ];

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkPipelineStageFlags waitStages [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = waitSemaphore != VK_NULL_HANDLE ? 1 : 0;
            submitInfo.pWaitSemaphores = &waitSemaphore;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            submitInfo.signalSemaphoreCount = signalSemaphore != VK_NULL_HANDLE ? 1 : 0;
            submitInfo.pSignalSemaphores = &signalSemaphore;

            VkResult result = vkQueueSubmit ( queue, 1, &submitInfo, fence );
            if (result != VK_SUCCESS)
                {
                throw std::runtime_error ( "Failed to submit command buffer" );
                }
            }
        }

    bool CEVulkanCommandBuffer::IsReadyForRecording () const
        {
        return m_Context != nullptr &&
            m_Context->GetDevice () != nullptr &&
            !m_CommandBuffers.empty () &&
            m_CurrentFrame < m_CommandBuffers.size ();
        }

    void CEVulkanCommandBuffer::CreateCommandPool ()
        {
        if (!m_Context || !m_Context->GetDevice ())
            {
            throw std::runtime_error ( "Invalid Vulkan context for command pool creation" );
            }

        QueueFamilyIndices queueFamilyIndices = m_Context->GetDevice ()->GetQueueFamilyIndices ();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value ();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult result = vkCreateCommandPool ( m_Context->GetDevice ()->GetDevice (), &poolInfo, nullptr, &m_CommandPool );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create command pool" );
            }

        CE_CORE_DEBUG ( "Command pool created successfully" );
        }

    void CEVulkanCommandBuffer::CreateCommandBuffers ()
        {
        if (!m_Context || !m_Context->GetDevice () || m_CommandPool == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Invalid state for command buffer creation" );
            }

        m_CommandBuffers.resize ( 2 ); // Standard double buffering

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast< uint32_t >( m_CommandBuffers.size () );

        VkResult result = vkAllocateCommandBuffers ( m_Context->GetDevice ()->GetDevice (), &allocInfo, m_CommandBuffers.data () );
        if (result != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to allocate command buffers" );
            }

        CE_CORE_DEBUG ( "Created {} command buffers", m_CommandBuffers.size () );
        }
    }