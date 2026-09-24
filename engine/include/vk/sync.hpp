#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace OLIA_ENGINE
{
    class Device;


    // ============================================================
    // Per Frame Synchronization
    // ============================================================

    struct FrameSync
    {
        VkSemaphore imageAvailable =
            VK_NULL_HANDLE;

        VkSemaphore renderFinished =
            VK_NULL_HANDLE;

        VkFence inFlight =
            VK_NULL_HANDLE;
    };


    // ============================================================
    // Sync
    // ============================================================

    class Sync
    {
    public:

        Sync();
        ~Sync();

        Sync(const Sync &) = delete;
        Sync &operator=(const Sync &) = delete;

        Sync(Sync &&) = delete;
        Sync &operator=(Sync &&) = delete;


        // ========================================================
        // Initialization
        // ========================================================

        bool Initialize(
            Device &device,
            uint32_t frameCount,
            uint32_t imageCount = 0
        );

        void Shutdown();

        bool RecreateRenderFinishedSemaphores(
            uint32_t imageCount
        );


        // ========================================================
        // Fence
        // ========================================================

        VkResult WaitForFrame(
            uint32_t frameIndex
        );

        VkResult ResetFrameFence(
            uint32_t frameIndex
        );


        // ========================================================
        // Getters
        // ========================================================

        VkSemaphore GetImageAvailable(
            uint32_t frameIndex
        ) const;

        VkSemaphore GetRenderFinished(
            uint32_t frameIndex
        ) const;

        VkFence GetInFlightFence(
            uint32_t frameIndex
        ) const;


        const std::vector<FrameSync> &
        GetFrames() const
        {
            return m_frames;
        }

        uint32_t GetFrameCount() const
        {
            return static_cast<uint32_t>(
                m_frames.size()
            );
        }

        bool IsInitialized() const
        {
            return !m_frames.empty();
        }


    private:

        bool CreateFrameSync();
        bool CreateRenderFinishedSemaphores(uint32_t count);


        Device *m_device = nullptr;

        std::vector<FrameSync> m_frames;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
    };

} // namespace OLIA_ENGINE