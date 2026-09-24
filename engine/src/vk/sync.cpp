#include "vk/sync.hpp"

#include "vk/device.hpp"

#include <iostream>

namespace OLIA_ENGINE
{
    // ============================================================
    // Constructor
    // ============================================================

    Sync::Sync()
    {
    }


    // ============================================================
    // Destructor
    // ============================================================

    Sync::~Sync()
    {
        Shutdown();
    }


    // ============================================================
    // Initialize
    // ============================================================

    bool Sync::Initialize(
        Device &device,
        uint32_t frameCount,
        uint32_t imageCount)
    {
        if (!device.IsInitialized())
        {
            std::cerr
                << "[Sync] Device is not initialized.\n";

            return false;
        }

        if (frameCount == 0)
        {
            return false;
        }


        m_device = &device;

        m_frames.resize(frameCount);


        if (!CreateFrameSync())
        {
            Shutdown();

            return false;
        }

        uint32_t renderFinishedCount = (imageCount > 0) ? imageCount : frameCount;
        if (!CreateRenderFinishedSemaphores(renderFinishedCount))
        {
            Shutdown();

            return false;
        }


        std::cout
            << "[Sync] Synchronization objects created. "
            << "Frames: "
            << frameCount
            << ", Image Semaphores: "
            << renderFinishedCount
            << '\n';


        return true;
    }


    // ============================================================
    // Shutdown
    // ============================================================

    void Sync::Shutdown()
    {
        if (m_device == nullptr)
        {
            return;
        }


        VkDevice device =
            m_device->GetDevice();


        if (device != VK_NULL_HANDLE)
        {
            for (VkSemaphore &semaphore :
                 m_renderFinishedSemaphores)
            {
                if (semaphore != VK_NULL_HANDLE)
                {
                    vkDestroySemaphore(
                        device,
                        semaphore,
                        nullptr
                    );
                }
            }

            for (FrameSync &frame :
                 m_frames)
            {
                if (frame.imageAvailable !=
                    VK_NULL_HANDLE)
                {
                    vkDestroySemaphore(
                        device,
                        frame.imageAvailable,
                        nullptr
                    );
                }


                if (frame.renderFinished !=
                    VK_NULL_HANDLE)
                {
                    vkDestroySemaphore(
                        device,
                        frame.renderFinished,
                        nullptr
                    );
                }


                if (frame.inFlight !=
                    VK_NULL_HANDLE)
                {
                    vkDestroyFence(
                        device,
                        frame.inFlight,
                        nullptr
                    );
                }


                frame.imageAvailable =
                    VK_NULL_HANDLE;

                frame.renderFinished =
                    VK_NULL_HANDLE;

                frame.inFlight =
                    VK_NULL_HANDLE;
            }
        }


        m_renderFinishedSemaphores.clear();
        m_frames.clear();

        m_device = nullptr;
    }


    // ============================================================
    // Create Frame Synchronization
    // ============================================================

    bool Sync::CreateFrameSync()
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


        VkSemaphoreCreateInfo semaphoreInfo{};

        semaphoreInfo.sType =
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        semaphoreInfo.pNext = nullptr;

        semaphoreInfo.flags = 0;


        VkFenceCreateInfo fenceInfo{};

        fenceInfo.sType =
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        fenceInfo.pNext = nullptr;

        // --------------------------------------------------------
        // IMPORTANT:
        //
        // The first frame must be allowed to proceed immediately.
        // --------------------------------------------------------

        fenceInfo.flags =
            VK_FENCE_CREATE_SIGNALED_BIT;


        for (FrameSync &frame :
             m_frames)
        {
            VkResult result =
                vkCreateSemaphore(
                    device,
                    &semaphoreInfo,
                    nullptr,
                    &frame.imageAvailable
                );

            if (result != VK_SUCCESS)
            {
                std::cerr
                    << "[Sync] Failed to create "
                    << "image available semaphore.\n";

                return false;
            }


            result =
                vkCreateSemaphore(
                    device,
                    &semaphoreInfo,
                    nullptr,
                    &frame.renderFinished
                );

            if (result != VK_SUCCESS)
            {
                std::cerr
                    << "[Sync] Failed to create "
                    << "render finished semaphore.\n";

                return false;
            }


            result =
                vkCreateFence(
                    device,
                    &fenceInfo,
                    nullptr,
                    &frame.inFlight
                );

            if (result != VK_SUCCESS)
            {
                std::cerr
                    << "[Sync] Failed to create "
                    << "in-flight fence.\n";

                return false;
            }
        }


        return true;
    }


    // ============================================================
    // Create Render Finished Semaphores (per swapchain image)
    // ============================================================

    bool Sync::CreateRenderFinishedSemaphores(uint32_t count)
    {
        if (m_device == nullptr || count == 0)
        {
            return false;
        }

        VkDevice device = m_device->GetDevice();
        if (device == VK_NULL_HANDLE)
        {
            return false;
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.flags = 0;

        m_renderFinishedSemaphores.resize(count, VK_NULL_HANDLE);
        for (uint32_t i = 0; i < count; ++i)
        {
            VkResult result = vkCreateSemaphore(
                device,
                &semaphoreInfo,
                nullptr,
                &m_renderFinishedSemaphores[i]
            );

            if (result != VK_SUCCESS)
            {
                std::cerr
                    << "[Sync] Failed to create render finished semaphore.\n";
                return false;
            }
        }

        return true;
    }


    // ============================================================
    // Recreate Render Finished Semaphores
    // ============================================================

    bool Sync::RecreateRenderFinishedSemaphores(uint32_t imageCount)
    {
        if (m_device == nullptr)
        {
            return false;
        }

        VkDevice device = m_device->GetDevice();
        for (VkSemaphore &semaphore : m_renderFinishedSemaphores)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(device, semaphore, nullptr);
                semaphore = VK_NULL_HANDLE;
            }
        }
        m_renderFinishedSemaphores.clear();

        return CreateRenderFinishedSemaphores(imageCount);
    }


    // ============================================================
    // Wait For Frame
    // ============================================================

    VkResult Sync::WaitForFrame(
        uint32_t frameIndex)
    {
        if (m_device == nullptr ||
            frameIndex >= m_frames.size())
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        VkDevice device =
            m_device->GetDevice();


        return vkWaitForFences(
            device,
            1,
            &m_frames[frameIndex].inFlight,
            VK_TRUE,
            UINT64_MAX
        );
    }


    // ============================================================
    // Reset Frame Fence
    // ============================================================

    VkResult Sync::ResetFrameFence(
        uint32_t frameIndex)
    {
        if (m_device == nullptr ||
            frameIndex >= m_frames.size())
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        VkDevice device =
            m_device->GetDevice();


        return vkResetFences(
            device,
            1,
            &m_frames[frameIndex].inFlight
        );
    }


    // ============================================================
    // Get Image Available Semaphore
    // ============================================================

    VkSemaphore Sync::GetImageAvailable(
        uint32_t frameIndex) const
    {
        if (frameIndex >= m_frames.size())
        {
            return VK_NULL_HANDLE;
        }

        return m_frames[frameIndex].imageAvailable;
    }


    // ============================================================
    // Get Render Finished Semaphore
    // ============================================================

    VkSemaphore Sync::GetRenderFinished(
        uint32_t imageIndex) const
    {
        if (imageIndex < m_renderFinishedSemaphores.size())
        {
            return m_renderFinishedSemaphores[imageIndex];
        }

        if (imageIndex < m_frames.size())
        {
            return m_frames[imageIndex].renderFinished;
        }

        return VK_NULL_HANDLE;
    }


    // ============================================================
    // Get In Flight Fence
    // ============================================================

    VkFence Sync::GetInFlightFence(
        uint32_t frameIndex) const
    {
        if (frameIndex >= m_frames.size())
        {
            return VK_NULL_HANDLE;
        }

        return m_frames[frameIndex].inFlight;
    }

} // namespace OLIA_ENGINE