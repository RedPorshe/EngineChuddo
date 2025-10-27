#include "Graphics/Vulkan/Core/CEVulkanCommandBuffer.hpp"
#include "Utils/Logger.hpp"
#include <stdexcept>

namespace CE
    {
    CEVulkanCommandBuffer::CEVulkanCommandBuffer ()
        {
        CE_CORE_DEBUG ( "Vulkan command buffer manager created" );
        }

    CEVulkanCommandBuffer::~CEVulkanCommandBuffer ()
        {
        Shutdown ();
        }

    bool CEVulkanCommandBuffer::Initialize ( CEVulkanContext * context, uint32_t maxFramesInFlight )
        {
        m_Context = context;

        try
            {
            CreateCommandPool ();
            CreateCommandBuffers ();

            // Ensure all command buffers are created properly
            for (auto cmdBuffer : m_CommandBuffers)
                {
                if (cmdBuffer == VK_NULL_HANDLE)
                    {
                    throw std::runtime_error ( "Command buffer creation failed!" );
                    }
                }

            CE_CORE_DEBUG ( "Vulkan command buffers initialized successfully ({} buffers)", maxFramesInFlight );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan command buffers: {}", e.what () );
                return false;
                }
        }

    void CEVulkanCommandBuffer::Shutdown ()
        {
        auto device = m_Context ? m_Context->GetDevice ()->GetDevice( ) : VK_NULL_HANDLE;
        if (!device) return;

        if (m_CommandPool != VK_NULL_HANDLE)
            {
            vkDestroyCommandPool ( device, m_CommandPool, nullptr );
            m_CommandPool = VK_NULL_HANDLE;
            }

        m_CommandBuffers.Clear ();
        CE_CORE_DEBUG ( "Vulkan command buffer manager shutdown complete" );
        }

    void CEVulkanCommandBuffer::CreateCommandPool ()
        {
        auto device = m_Context->GetDevice ()->GetDevice();
        auto queueIndices = m_Context->GetDevice()->GetQueueFamilyIndices ();

        VkCommandPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueIndices.graphicsFamily.value ();

        if (vkCreateCommandPool ( device, &poolInfo, nullptr, &m_CommandPool ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to create command pool!" );
            }

        CE_CORE_DEBUG ( "Command pool created" );
        }

    void CEVulkanCommandBuffer::CreateCommandBuffers ()
        {
        auto device = m_Context->GetDevice ()->GetDevice();

        // Create one command buffer per frame in flight
        m_CommandBuffers.Resize ( 2 ); // Standard double buffering

        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast< uint32_t >( m_CommandBuffers.Size () );

        if (vkAllocateCommandBuffers ( device, &allocInfo, m_CommandBuffers.RawData () ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to allocate command buffers!" );
            }

        CE_CORE_DEBUG ( "Command buffers allocated" );
        }

    void CEVulkanCommandBuffer::ResetCurrent ()
        {
        if (m_CommandBuffers[ m_CurrentFrame ] != VK_NULL_HANDLE)
            {
            vkResetCommandBuffer ( m_CommandBuffers[ m_CurrentFrame ], 0 );
            }
        }

    void CEVulkanCommandBuffer::BeginRecording ()
        {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer ( m_CommandBuffers[ m_CurrentFrame ], &beginInfo ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to begin recording command buffer!" );
            }
        }

    void CEVulkanCommandBuffer::EndRecording ()
        {
        if (vkEndCommandBuffer ( m_CommandBuffers[ m_CurrentFrame ] ) != VK_SUCCESS)
            {
            throw std::runtime_error ( "Failed to record command buffer!" );
            }
        }

    bool CEVulkanCommandBuffer::IsReadyForRecording () const
        {
        return m_CommandPool != VK_NULL_HANDLE &&
            m_CurrentFrame < m_CommandBuffers.Size () &&
            m_CommandBuffers[ m_CurrentFrame ] != VK_NULL_HANDLE;
        }
    }