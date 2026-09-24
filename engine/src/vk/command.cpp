#include "vk/command.hpp"

#include "vk/device.hpp"

#include <iostream>

namespace OLIA_ENGINE
{
    // ============================================================
    // Constructor
    // ============================================================

    Command::Command()
    {
    }


    // ============================================================
    // Destructor
    // ============================================================

    Command::~Command()
    {
        Shutdown();
    }


    // ============================================================
    // Initialize
    // ============================================================

    bool Command::Initialize(
        Device &device,
        uint32_t frameCount)
    {
        if (!device.IsInitialized())
        {
            std::cerr
                << "[Command] Device is not initialized.\n";

            return false;
        }

        if (frameCount == 0)
        {
            std::cerr
                << "[Command] Invalid frame count.\n";

            return false;
        }


        m_device = &device;

        m_commandBuffers.resize(frameCount);


        // --------------------------------------------------------
        // Create Command Pool
        // --------------------------------------------------------

        if (!CreateCommandPool())
        {
            Shutdown();

            return false;
        }


        // --------------------------------------------------------
        // Allocate Command Buffers
        // --------------------------------------------------------

        if (!CreateCommandBuffers())
        {
            Shutdown();

            return false;
        }


        std::cout
            << "[Command] Command pool created.\n";

        std::cout
            << "[Command] Command buffers: "
            << m_commandBuffers.size()
            << '\n';


        return true;
    }


    // ============================================================
    // Shutdown
    // ============================================================

    void Command::Shutdown()
    {
        if (m_device == nullptr)
        {
            return;
        }


        VkDevice device =
            m_device->GetDevice();


        if (device != VK_NULL_HANDLE)
        {
            if (m_commandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(
                    device,
                    m_commandPool,
                    nullptr
                );
            }
        }


        m_commandPool =
            VK_NULL_HANDLE;

        m_commandBuffers.clear();

        m_device = nullptr;
    }


    // ============================================================
    // Create Command Pool
    // ============================================================

    bool Command::CreateCommandPool()
    {
        if (m_device == nullptr)
        {
            return false;
        }


        VkDevice device =
            m_device->GetDevice();


        if (device == VK_NULL_HANDLE)
        {
            return false;
        }


        VkCommandPoolCreateInfo createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        createInfo.pNext = nullptr;

        createInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        createInfo.queueFamilyIndex =
            m_device->GetGraphicsQueueFamily();


        VkResult result =
            vkCreateCommandPool(
                device,
                &createInfo,
                nullptr,
                &m_commandPool
            );


        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Command] Failed to create command pool. "
                << "VkResult: "
                << result
                << '\n';

            m_commandPool =
                VK_NULL_HANDLE;

            return false;
        }


        return true;
    }


    // ============================================================
    // Create Command Buffers
    // ============================================================

    bool Command::CreateCommandBuffers()
    {
        if (m_device == nullptr ||
            m_commandPool == VK_NULL_HANDLE)
        {
            return false;
        }


        VkDevice device =
            m_device->GetDevice();


        // --------------------------------------------------------
        // Use Existing Frame Count
        //
        // The caller must have resized m_commandBuffers.
        // --------------------------------------------------------

        if (m_commandBuffers.empty())
        {
            std::cerr
                << "[Command] No command buffers requested.\n";

            return false;
        }


        VkCommandBufferAllocateInfo allocateInfo{};

        allocateInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        allocateInfo.pNext = nullptr;

        allocateInfo.commandPool =
            m_commandPool;

        allocateInfo.level =
            VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        allocateInfo.commandBufferCount =
            static_cast<uint32_t>(
                m_commandBuffers.size()
            );


        VkResult result =
            vkAllocateCommandBuffers(
                device,
                &allocateInfo,
                m_commandBuffers.data()
            );


        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Command] Failed to allocate command buffers. "
                << "VkResult: "
                << result
                << '\n';

            m_commandBuffers.clear();

            return false;
        }


        return true;
    }


    // ============================================================
    // Begin Command Buffer
    // ============================================================

    bool Command::Begin(
        uint32_t frameIndex,
        VkCommandBufferUsageFlags flags)
    {
        if (frameIndex >=
            m_commandBuffers.size())
        {
            return false;
        }


        VkCommandBuffer commandBuffer =
            m_commandBuffers[frameIndex];


        VkCommandBufferBeginInfo beginInfo{};

        beginInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        beginInfo.pNext = nullptr;

        beginInfo.flags = flags;

        beginInfo.pInheritanceInfo = nullptr;


        VkResult result =
            vkBeginCommandBuffer(
                commandBuffer,
                &beginInfo
            );


        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Command] Failed to begin command buffer. "
                << "VkResult: "
                << result
                << '\n';

            return false;
        }


        return true;
    }


    // ============================================================
    // End Command Buffer
    // ============================================================

    bool Command::End(
        uint32_t frameIndex)
    {
        if (frameIndex >=
            m_commandBuffers.size())
        {
            return false;
        }


        VkResult result =
            vkEndCommandBuffer(
                m_commandBuffers[frameIndex]
            );


        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Command] Failed to end command buffer. "
                << "VkResult: "
                << result
                << '\n';

            return false;
        }


        return true;
    }


    // ============================================================
    // Reset Command Buffer
    // ============================================================

    void Command::Reset(
        uint32_t frameIndex)
    {
        if (frameIndex >=
            m_commandBuffers.size())
        {
            return;
        }


        vkResetCommandBuffer(
            m_commandBuffers[frameIndex],
            0
        );
    }


    // ============================================================
    // Begin Single Time Commands
    // ============================================================

    VkCommandBuffer Command::BeginSingleTimeCommands()
    {
        if (m_device == nullptr || m_commandPool == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Command] Device or command pool not initialized for single time commands.\n";
            return VK_NULL_HANDLE;
        }

        VkDevice device = m_device->GetDevice();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Command] Failed to allocate single time command buffer. VkResult: "
                << result << '\n';
            return VK_NULL_HANDLE;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Command] Failed to begin single time command buffer. VkResult: "
                << result << '\n';
            vkFreeCommandBuffers(device, m_commandPool, 1, &commandBuffer);
            return VK_NULL_HANDLE;
        }

        return commandBuffer;
    }


    // ============================================================
    // End Single Time Commands
    // ============================================================

    void Command::EndSingleTimeCommands(
        VkCommandBuffer commandBuffer)
    {
        if (m_device == nullptr || m_commandPool == VK_NULL_HANDLE || commandBuffer == VK_NULL_HANDLE)
        {
            return;
        }

        VkDevice device = m_device->GetDevice();
        VkQueue graphicsQueue = m_device->GetGraphicsQueue();

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, m_commandPool, 1, &commandBuffer);
    }

} // namespace OLIA_ENGINE