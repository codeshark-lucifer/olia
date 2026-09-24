#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace OLIA_ENGINE
{
    class Device;

    // ============================================================
    // SwapChain Support Details
    // ============================================================

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};

        std::vector<VkSurfaceFormatKHR> formats;

        std::vector<VkPresentModeKHR> presentModes;
    };


    // ============================================================
    // SwapChain
    // ============================================================

    class SwapChain
    {
    public:

        SwapChain();
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        SwapChain(SwapChain &&) = delete;
        SwapChain &operator=(SwapChain &&) = delete;


        // ========================================================
        // Initialization
        // ========================================================

        bool Initialize(
            Device &device,
            uint32_t windowWidth,
            uint32_t windowHeight
        );

        void Shutdown();


        // ========================================================
        // Recreation
        // ========================================================

        bool Recreate(
            uint32_t windowWidth,
            uint32_t windowHeight
        );


        // ========================================================
        // Image Acquisition
        // ========================================================

        VkResult AcquireNextImage(
            VkSemaphore imageAvailableSemaphore,
            uint32_t *imageIndex
        );


        // ========================================================
        // Presentation
        // ========================================================

        VkResult Present(
            uint32_t imageIndex,
            VkSemaphore renderFinishedSemaphore
        );


        // ========================================================
        // Support Query
        // ========================================================

        SwapChainSupportDetails QuerySupport(
            VkPhysicalDevice device
        ) const;


        // ========================================================
        // Format Selection
        // ========================================================

        VkSurfaceFormatKHR ChooseSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats
        ) const;


        // ========================================================
        // Present Mode Selection
        // ========================================================

        VkPresentModeKHR ChoosePresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes
        ) const;


        // ========================================================
        // Extent Selection
        // ========================================================

        VkExtent2D ChooseExtent(
            const VkSurfaceCapabilitiesKHR &capabilities,
            uint32_t windowWidth,
            uint32_t windowHeight
        ) const;


        // ========================================================
        // Getters
        // ========================================================

        VkSwapchainKHR GetSwapChain() const
        {
            return m_swapChain;
        }

        VkFormat GetImageFormat() const
        {
            return m_imageFormat;
        }

        VkColorSpaceKHR GetColorSpace() const
        {
            return m_colorSpace;
        }

        VkExtent2D GetExtent() const
        {
            return m_extent;
        }

        uint32_t GetImageCount() const
        {
            return static_cast<uint32_t>(
                m_images.size()
            );
        }

        const std::vector<VkImage> &GetImages() const
        {
            return m_images;
        }

        const std::vector<VkImageView> &GetImageViews() const
        {
            return m_imageViews;
        }

        VkImage GetImage(uint32_t index) const
        {
            return m_images.at(index);
        }

        VkImageView GetImageView(uint32_t index) const
        {
            return m_imageViews.at(index);
        }

        Device *GetDevice() const
        {
            return m_device;
        }

        bool IsInitialized() const
        {
            return m_swapChain != VK_NULL_HANDLE;
        }


    private:

        // ========================================================
        // Creation
        // ========================================================

        bool CreateSwapChain(
            uint32_t windowWidth,
            uint32_t windowHeight
        );

        bool CreateImageViews();


        // ========================================================
        // Destruction
        // ========================================================

        void DestroyImageViews();

        void DestroySwapChain();


        // ========================================================
        // Vulkan Objects
        // ========================================================

        VkSwapchainKHR m_swapChain =
            VK_NULL_HANDLE;


        std::vector<VkImage> m_images;

        std::vector<VkImageView> m_imageViews;


        // ========================================================
        // Swapchain Properties
        // ========================================================

        VkFormat m_imageFormat =
            VK_FORMAT_UNDEFINED;

        VkColorSpaceKHR m_colorSpace =
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        VkExtent2D m_extent{};


        // ========================================================
        // Device
        // ========================================================

        Device *m_device = nullptr;


        // ========================================================
        // Window Size
        // ========================================================

        uint32_t m_windowWidth = 0;

        uint32_t m_windowHeight = 0;
    };

} // namespace OLIA_ENGINE