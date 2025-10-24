#include "CEVulkanCommandBuffer.hpp"
#include "Core/Logger.h"
#include <stdexcept>

namespace CE
    {
    CEVulkanCommandBuffer::CEVulkanCommandBuffer ( CEVulkanContext * context )
        : Context ( context )
        {
        CE_CORE_DEBUG ( "Vulkan command buffer created" );
        }

    CEVulkanCommandBuffer::~CEVulkanCommandBuffer ()
        {
        Shutdown ();
        }

    bool CEVulkanCommandBuffer::Initialize ()
        {
        try
            {
            CreateCommandPool ();
            CreateCommandBuffer ();

            // Проверяем, что command buffer создан правильно
            if (CommandBuffer == VK_NULL_HANDLE)
                {
                throw std::runtime_error ( "Command buffer is null after creation!" );
                }

            CE_CORE_DEBUG ( "Vulkan command buffer initialized successfully (handle: {})", ( void * ) CommandBuffer );
            return true;
            }
            catch (const std::exception & e)
                {
                CE_CORE_ERROR ( "Failed to initialize Vulkan command buffer: {}", e.what () );
                return false;
                }
        }

    void CEVulkanCommandBuffer::Shutdown ()
        {
        auto device = Context ? Context->GetDevice () : VK_NULL_HANDLE;

        if (device == VK_NULL_HANDLE)
            {
            CE_CORE_DEBUG ( "No valid device for command buffer shutdown" );
            return;
            }

        if (CommandBuffer != VK_NULL_HANDLE)
            {
                // Command buffers are freed when pool is destroyed
            CommandBuffer = VK_NULL_HANDLE;
            }

        if (CommandPool != VK_NULL_HANDLE)
            {
            vkDestroyCommandPool ( device, CommandPool, nullptr );
            CommandPool = VK_NULL_HANDLE;
            }

        CE_CORE_DEBUG ( "Vulkan command buffer shutdown complete" );
        }

    void CEVulkanCommandBuffer::CreateCommandPool ()
        {
        auto device = Context->GetDevice ();
        auto queueIndices = Context->GetQueueFamilyIndices ();

        VkCommandPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueIndices.graphicsFamily.value ();

        VkResult result = vkCreateCommandPool ( device, &poolInfo, nullptr, &CommandPool );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to create command pool: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to create command pool!" );
            }

        CE_CORE_DEBUG ( "Command pool created" );
        }

    void CEVulkanCommandBuffer::CreateCommandBuffer ()
        {
        auto device = Context->GetDevice ();

        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkResult result = vkAllocateCommandBuffers ( device, &allocInfo, &CommandBuffer );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to allocate command buffers: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to allocate command buffers!" );
            }

        CE_CORE_DEBUG ( "Command buffer allocated" );
        }

    void CEVulkanCommandBuffer::BeginRecording ()
        {
        if (CommandBuffer == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Command buffer is null!" );
            }

            
        ValidateRecordingState ();

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer ( CommandBuffer, &beginInfo );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to begin recording command buffer: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to begin recording command buffer!" );
            }

        
        }

   

    void CEVulkanCommandBuffer::ValidateRecordingState () const
        {
        if (CommandBuffer == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Command buffer is null during validation!" );
            }

        if (CommandPool == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Command pool is null during validation!" );
            }        
        }

    void CEVulkanCommandBuffer::EndRecording ()
        {
        if (CommandBuffer == VK_NULL_HANDLE)
            {
            throw std::runtime_error ( "Command buffer is null!" );
            }

        VkResult result = vkEndCommandBuffer ( CommandBuffer );
        if (result != VK_SUCCESS)
            {
            CE_CORE_ERROR ( "Failed to end recording command buffer: {}", static_cast< int >( result ) );
            throw std::runtime_error ( "Failed to record command buffer!" );
            }       
        }

    void CEVulkanCommandBuffer::Reset ()
        {
        if (CommandBuffer == VK_NULL_HANDLE)
            {
            return;
            }

        VkResult result = vkResetCommandBuffer ( CommandBuffer, 0 );
        if (result != VK_SUCCESS)
            {
            CE_CORE_WARN ( "Failed to reset command buffer: {}, but continuing...", static_cast< int >( result ) );
            // Не бросаем исключение здесь, так как это может быть нормальной ситуацией
            }       
        }

    bool CEVulkanCommandBuffer::IsReadyForRecording () const
        {
        if (CommandBuffer == VK_NULL_HANDLE)
            {
            CE_CORE_ERROR ( "Command buffer is null" );
            return false;
            }

        if (CommandPool == VK_NULL_HANDLE)
            {
            CE_CORE_ERROR ( "Command pool is null" );
            return false;
            }

            // Дополнительные проверки можно добавить здесь
        return true;
        }
    }