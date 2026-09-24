#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace OLIA_ENGINE
{
    class Device;

    // ============================================================
    // Command
    //
    // Owns:
    //     VkCommandPool
    //     VkCommandBuffers
    //
    // One command buffer is allocated for each frame-in-flight.
    // ============================================================

    class Command
    {
    public:

        Command();
        ~Command();

        Command(const Command &) = delete;
        Command &operator=(const Command &) = delete;

        Command(Command &&) = delete;
        Command &operator=(Command &&) = delete;


        // ========================================================
        // Initialization
        // ========================================================

        bool Initialize(
            Device &device,
            uint32_t frameCount
        );

        void Shutdown();


        // ========================================================
        // Command Buffer
        // ========================================================

        bool Begin(
            uint32_t frameIndex,
            VkCommandBufferUsageFlags flags = 0
        );

        bool End(uint32_t frameIndex);


        void Reset(
            uint32_t frameIndex
        );


        // ========================================================
        // Single-Time Commands
        // ========================================================

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(
            VkCommandBuffer commandBuffer
        );


        // ========================================================
        // Getters
        // ========================================================

        VkCommandPool GetCommandPool() const
        {
            return m_commandPool;
        }

        VkCommandBuffer GetCommandBuffer(
            uint32_t frameIndex
        ) const
        {
            if (frameIndex >= m_commandBuffers.size())
            {
                return VK_NULL_HANDLE;
            }

            return m_commandBuffers[frameIndex];
        }

        const std::vector<VkCommandBuffer> &
        GetCommandBuffers() const
        {
            return m_commandBuffers;
        }

        uint32_t GetFrameCount() const
        {
            return static_cast<uint32_t>(
                m_commandBuffers.size()
            );
        }

        bool IsInitialized() const
        {
            return m_commandPool != VK_NULL_HANDLE;
        }


    private:

        // ========================================================
        // Creation
        // ========================================================

        bool CreateCommandPool();

        bool CreateCommandBuffers();


        // ========================================================
        // Vulkan
        // ========================================================

        VkCommandPool m_commandPool =
            VK_NULL_HANDLE;

        std::vector<VkCommandBuffer>
            m_commandBuffers;


        // ========================================================
        // Device
        // ========================================================

        Device *m_device = nullptr;
    };

} // namespace OLIA_ENGINE