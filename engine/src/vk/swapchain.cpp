#include "vk/swapchain.hpp"

#include "vk/device.hpp"
#include "vk/vkcontext.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace OLIA_ENGINE
{
    // ============================================================
    // Constructor
    // ============================================================

    SwapChain::SwapChain()
    {
    }


    // ============================================================
    // Destructor
    // ============================================================

    SwapChain::~SwapChain()
    {
        Shutdown();
    }


    // ============================================================
    // Initialize
    // ============================================================

    bool SwapChain::Initialize(
        Device &device,
        uint32_t windowWidth,
        uint32_t windowHeight)
    {
        if (!device.IsInitialized())
        {
            std::cerr
                << "[SwapChain] Device is not initialized.\n";

            return false;
        }

        if (vk_context.surface == VK_NULL_HANDLE)
        {
            std::cerr
                << "[SwapChain] Vulkan surface is null.\n";

            return false;
        }

        if (windowWidth == 0 || windowHeight == 0)
        {
            std::cerr
                << "[SwapChain] Invalid window size.\n";

            return false;
        }


        m_device = &device;

        m_windowWidth = windowWidth;
        m_windowHeight = windowHeight;


        // --------------------------------------------------------
        // Create Swapchain
        // --------------------------------------------------------

        if (!CreateSwapChain(
                windowWidth,
                windowHeight))
        {
            std::cerr
                << "[SwapChain] Failed to create swapchain.\n";

            m_device = nullptr;

            return false;
        }


        // --------------------------------------------------------
        // Create Image Views
        // --------------------------------------------------------

        if (!CreateImageViews())
        {
            std::cerr
                << "[SwapChain] Failed to create image views.\n";

            Shutdown();

            return false;
        }


        std::cout
            << "[SwapChain] Created successfully.\n";

        std::cout
            << "[SwapChain] Images: "
            << m_images.size()
            << '\n';

        std::cout
            << "[SwapChain] Extent: "
            << m_extent.width
            << "x"
            << m_extent.height
            << '\n';

        std::cout
            << "[SwapChain] Format: "
            << m_imageFormat
            << '\n';


        return true;
    }


    // ============================================================
    // Shutdown
    // ============================================================

    void SwapChain::Shutdown()
    {
        if (m_device == nullptr)
        {
            return;
        }

        VkDevice device =
            m_device->GetDevice();

        if (device != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(device);
        }


        DestroyImageViews();

        DestroySwapChain();


        m_device = nullptr;

        m_images.clear();
        m_imageViews.clear();

        m_imageFormat = VK_FORMAT_UNDEFINED;

        m_colorSpace =
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        m_extent = {};

        m_windowWidth = 0;
        m_windowHeight = 0;
    }


    // ============================================================
    // Recreate
    // ============================================================

    bool SwapChain::Recreate(
        uint32_t windowWidth,
        uint32_t windowHeight)
    {
        if (m_device == nullptr)
        {
            std::cerr
                << "[SwapChain] Cannot recreate: "
                << "device is null.\n";

            return false;
        }

        if (windowWidth == 0 || windowHeight == 0)
        {
            return false;
        }


        VkDevice device =
            m_device->GetDevice();

        if (device == VK_NULL_HANDLE)
        {
            return false;
        }


        // --------------------------------------------------------
        // Wait For GPU
        // --------------------------------------------------------

        VkResult result =
            vkDeviceWaitIdle(device);

        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[SwapChain] vkDeviceWaitIdle failed. "
                << "VkResult: "
                << result
                << '\n';

            return false;
        }


        // --------------------------------------------------------
        // Destroy Old Swapchain
        // --------------------------------------------------------

        DestroyImageViews();

        DestroySwapChain();


        // --------------------------------------------------------
        // Update Window Size
        // --------------------------------------------------------

        m_windowWidth = windowWidth;
        m_windowHeight = windowHeight;


        // --------------------------------------------------------
        // Create New Swapchain
        // --------------------------------------------------------

        if (!CreateSwapChain(
                windowWidth,
                windowHeight))
        {
            std::cerr
                << "[SwapChain] Failed to recreate swapchain.\n";

            return false;
        }


        // --------------------------------------------------------
        // Create New Image Views
        // --------------------------------------------------------

        if (!CreateImageViews())
        {
            std::cerr
                << "[SwapChain] Failed to recreate image views.\n";

            DestroySwapChain();

            return false;
        }


        std::cout
            << "[SwapChain] Recreated: "
            << m_extent.width
            << "x"
            << m_extent.height
            << '\n';


        return true;
    }


    // ============================================================
    // Query Swapchain Support
    // ============================================================

    SwapChainSupportDetails SwapChain::QuerySupport(
        VkPhysicalDevice device) const
    {
        SwapChainSupportDetails details{};

        if (device == VK_NULL_HANDLE)
        {
            return details;
        }

        if (vk_context.surface == VK_NULL_HANDLE)
        {
            return details;
        }


        // --------------------------------------------------------
        // Surface Capabilities
        // --------------------------------------------------------

        VkResult result =
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                device,
                vk_context.surface,
                &details.capabilities
            );

        if (result != VK_SUCCESS)
        {
            return details;
        }


        // --------------------------------------------------------
        // Surface Formats
        // --------------------------------------------------------

        uint32_t formatCount = 0;

        result =
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                vk_context.surface,
                &formatCount,
                nullptr
            );

        if (result != VK_SUCCESS)
        {
            return details;
        }

        if (formatCount > 0)
        {
            details.formats.resize(formatCount);

            result =
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device,
                    vk_context.surface,
                    &formatCount,
                    details.formats.data()
                );

            if (result != VK_SUCCESS)
            {
                details.formats.clear();

                return details;
            }
        }


        // --------------------------------------------------------
        // Present Modes
        // --------------------------------------------------------

        uint32_t presentModeCount = 0;

        result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                vk_context.surface,
                &presentModeCount,
                nullptr
            );

        if (result != VK_SUCCESS)
        {
            return details;
        }

        if (presentModeCount > 0)
        {
            details.presentModes.resize(
                presentModeCount
            );

            result =
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device,
                    vk_context.surface,
                    &presentModeCount,
                    details.presentModes.data()
                );

            if (result != VK_SUCCESS)
            {
                details.presentModes.clear();

                return details;
            }
        }


        return details;
    }


    // ============================================================
    // Choose Surface Format
    // ============================================================

    VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats)
        const
    {
        if (availableFormats.empty())
        {
            return {};
        }


        // --------------------------------------------------------
        // Preferred Format
        // --------------------------------------------------------

        for (const auto &format : availableFormats)
        {
            if ((format.format == VK_FORMAT_B8G8R8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_UNORM) &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }


        // --------------------------------------------------------
        // Secondary Format: SRGB
        // --------------------------------------------------------

        for (const auto &format : availableFormats)
        {
            if ((format.format == VK_FORMAT_B8G8R8A8_SRGB || format.format == VK_FORMAT_R8G8B8A8_SRGB) &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }


        // --------------------------------------------------------
        // Fallback
        // --------------------------------------------------------

        return availableFormats[0];
    }


    // ============================================================
    // Choose Present Mode
    // ============================================================

    VkPresentModeKHR SwapChain::ChoosePresentMode(
        const std::vector<VkPresentModeKHR>
            &availablePresentModes)
        const
    {
        // --------------------------------------------------------
        // Prefer Mailbox
        // --------------------------------------------------------

        for (const auto &presentMode :
             availablePresentModes)
        {
            if (presentMode ==
                VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return presentMode;
            }
        }


        // --------------------------------------------------------
        // Fallback To FIFO
        // --------------------------------------------------------

        return VK_PRESENT_MODE_FIFO_KHR;
    }


    // ============================================================
    // Choose Extent
    // ============================================================

    VkExtent2D SwapChain::ChooseExtent(
        const VkSurfaceCapabilitiesKHR &capabilities,
        uint32_t windowWidth,
        uint32_t windowHeight)
        const
    {
        // --------------------------------------------------------
        // Surface Chooses Extent
        // --------------------------------------------------------

        if (capabilities.currentExtent.width !=
                std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }


        // --------------------------------------------------------
        // Window Size
        // --------------------------------------------------------

        VkExtent2D actualExtent{};

        actualExtent.width = windowWidth;
        actualExtent.height = windowHeight;


        // --------------------------------------------------------
        // Clamp Width
        // --------------------------------------------------------

        actualExtent.width =
            std::clamp(
                actualExtent.width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width
            );


        // --------------------------------------------------------
        // Clamp Height
        // --------------------------------------------------------

        actualExtent.height =
            std::clamp(
                actualExtent.height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height
            );


        return actualExtent;
    }


    // ============================================================
    // Create Swapchain
    // ============================================================

    bool SwapChain::CreateSwapChain(
        uint32_t windowWidth,
        uint32_t windowHeight)
    {
        if (m_device == nullptr)
        {
            return false;
        }


        VkDevice device =
            m_device->GetDevice();

        VkPhysicalDevice physicalDevice =
            m_device->GetPhysicalDevice();


        if (device == VK_NULL_HANDLE ||
            physicalDevice == VK_NULL_HANDLE)
        {
            return false;
        }


        // --------------------------------------------------------
        // Query Support
        // --------------------------------------------------------

        SwapChainSupportDetails support =
            QuerySupport(physicalDevice);


        if (support.formats.empty())
        {
            std::cerr
                << "[SwapChain] No surface formats available.\n";

            return false;
        }


        if (support.presentModes.empty())
        {
            std::cerr
                << "[SwapChain] No present modes available.\n";

            return false;
        }


        // --------------------------------------------------------
        // Select Format
        // --------------------------------------------------------

        VkSurfaceFormatKHR surfaceFormat =
            ChooseSurfaceFormat(
                support.formats
            );


        // --------------------------------------------------------
        // Select Present Mode
        // --------------------------------------------------------

        VkPresentModeKHR presentMode =
            ChoosePresentMode(
                support.presentModes
            );


        // --------------------------------------------------------
        // Select Extent
        // --------------------------------------------------------

        VkExtent2D extent =
            ChooseExtent(
                support.capabilities,
                windowWidth,
                windowHeight
            );


        // --------------------------------------------------------
        // Image Count
        // --------------------------------------------------------

        uint32_t imageCount =
            support.capabilities.minImageCount + 1;


        if (support.capabilities.maxImageCount > 0 &&
            imageCount >
                support.capabilities.maxImageCount)
        {
            imageCount =
                support.capabilities.maxImageCount;
        }


        // --------------------------------------------------------
        // Swapchain Create Info
        // --------------------------------------------------------

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        createInfo.pNext = nullptr;

        createInfo.surface =
            vk_context.surface;

        createInfo.minImageCount =
            imageCount;

        createInfo.imageFormat =
            surfaceFormat.format;

        createInfo.imageColorSpace =
            surfaceFormat.colorSpace;

        createInfo.imageExtent =
            extent;

        createInfo.imageArrayLayers = 1;

        createInfo.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT;


        // --------------------------------------------------------
        // Queue Families
        // --------------------------------------------------------

        const QueueFamilyIndices &queueFamilies =
            m_device->GetQueueFamilies();


        uint32_t queueFamilyIndices[] =
        {
            queueFamilies.graphicsFamily,
            queueFamilies.presentFamily
        };


        if (queueFamilies.graphicsFamily !=
            queueFamilies.presentFamily)
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_CONCURRENT;

            createInfo.queueFamilyIndexCount = 2;

            createInfo.pQueueFamilyIndices =
                queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_EXCLUSIVE;

            createInfo.queueFamilyIndexCount = 0;

            createInfo.pQueueFamilyIndices = nullptr;
        }


        // --------------------------------------------------------
        // Transform
        // --------------------------------------------------------

        createInfo.preTransform =
            support.capabilities.currentTransform;


        // --------------------------------------------------------
        // Alpha
        // --------------------------------------------------------

        createInfo.compositeAlpha =
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;


        // --------------------------------------------------------
        // Present Mode
        // --------------------------------------------------------

        createInfo.presentMode =
            presentMode;


        // --------------------------------------------------------
        // Clipping
        // --------------------------------------------------------

        createInfo.clipped = VK_TRUE;


        // --------------------------------------------------------
        // Old Swapchain
        // --------------------------------------------------------

        createInfo.oldSwapchain =
            VK_NULL_HANDLE;


        // --------------------------------------------------------
        // Create
        // --------------------------------------------------------

        VkSwapchainKHR newSwapChain =
            VK_NULL_HANDLE;

        VkResult result =
            vkCreateSwapchainKHR(
                device,
                &createInfo,
                nullptr,
                &newSwapChain
            );

        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[SwapChain] vkCreateSwapchainKHR failed. "
                << "VkResult: "
                << result
                << '\n';

            return false;
        }


        m_swapChain =
            newSwapChain;


        // --------------------------------------------------------
        // Save Properties
        // --------------------------------------------------------

        m_imageFormat =
            surfaceFormat.format;

        m_colorSpace =
            surfaceFormat.colorSpace;

        m_extent =
            extent;


        // --------------------------------------------------------
        // Retrieve Images
        // --------------------------------------------------------

        uint32_t actualImageCount = 0;

        result =
            vkGetSwapchainImagesKHR(
                device,
                m_swapChain,
                &actualImageCount,
                nullptr
            );

        if (result != VK_SUCCESS ||
            actualImageCount == 0)
        {
            std::cerr
                << "[SwapChain] Failed to get swapchain images.\n";

            DestroySwapChain();

            return false;
        }


        m_images.resize(actualImageCount);

        result =
            vkGetSwapchainImagesKHR(
                device,
                m_swapChain,
                &actualImageCount,
                m_images.data()
            );

        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[SwapChain] Failed to retrieve "
                << "swapchain images.\n";

            DestroySwapChain();

            return false;
        }


        return true;
    }


    // ============================================================
    // Create Image Views
    // ============================================================

    bool SwapChain::CreateImageViews()
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


        m_imageViews.resize(
            m_images.size(),
            VK_NULL_HANDLE
        );


        for (size_t i = 0; i < m_images.size(); ++i)
        {
            VkImageViewCreateInfo createInfo{};

            createInfo.sType =
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            createInfo.pNext = nullptr;

            createInfo.image =
                m_images[i];

            createInfo.viewType =
                VK_IMAGE_VIEW_TYPE_2D;

            createInfo.format =
                m_imageFormat;


            // ----------------------------------------------------
            // Color Components
            // ----------------------------------------------------

            createInfo.components.r =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.g =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.b =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.a =
                VK_COMPONENT_SWIZZLE_IDENTITY;


            // ----------------------------------------------------
            // Subresource Range
            // ----------------------------------------------------

            createInfo.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            createInfo.subresourceRange.baseMipLevel =
                0;

            createInfo.subresourceRange.levelCount =
                1;

            createInfo.subresourceRange.baseArrayLayer =
                0;

            createInfo.subresourceRange.layerCount =
                1;


            // ----------------------------------------------------
            // Create Image View
            // ----------------------------------------------------

            VkResult result =
                vkCreateImageView(
                    device,
                    &createInfo,
                    nullptr,
                    &m_imageViews[i]
                );

            if (result != VK_SUCCESS)
            {
                std::cerr
                    << "[SwapChain] Failed to create "
                    << "image view "
                    << i
                    << ". VkResult: "
                    << result
                    << '\n';

                DestroyImageViews();

                return false;
            }
        }


        return true;
    }


    // ============================================================
    // Destroy Image Views
    // ============================================================

    void SwapChain::DestroyImageViews()
    {
        if (m_device == nullptr)
        {
            return;
        }


        VkDevice device =
            m_device->GetDevice();


        if (device == VK_NULL_HANDLE)
        {
            return;
        }


        for (VkImageView imageView :
             m_imageViews)
        {
            if (imageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(
                    device,
                    imageView,
                    nullptr
                );
            }
        }


        m_imageViews.clear();
    }


    // ============================================================
    // Destroy Swapchain
    // ============================================================

    void SwapChain::DestroySwapChain()
    {
        if (m_device == nullptr)
        {
            return;
        }


        VkDevice device =
            m_device->GetDevice();


        if (device == VK_NULL_HANDLE)
        {
            return;
        }


        if (m_swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(
                device,
                m_swapChain,
                nullptr
            );

            m_swapChain =
                VK_NULL_HANDLE;
        }


        m_images.clear();
    }


    // ============================================================
    // Acquire Next Image
    // ============================================================

    VkResult SwapChain::AcquireNextImage(
        VkSemaphore imageAvailableSemaphore,
        uint32_t *imageIndex)
    {
        if (m_device == nullptr ||
            m_swapChain == VK_NULL_HANDLE ||
            imageIndex == nullptr)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        if (imageAvailableSemaphore ==
            VK_NULL_HANDLE)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        VkDevice device =
            m_device->GetDevice();


        return vkAcquireNextImageKHR(
            device,
            m_swapChain,
            UINT64_MAX,
            imageAvailableSemaphore,
            VK_NULL_HANDLE,
            imageIndex
        );
    }


    // ============================================================
    // Present
    // ============================================================

    VkResult SwapChain::Present(
        uint32_t imageIndex,
        VkSemaphore renderFinishedSemaphore)
    {
        if (m_device == nullptr ||
            m_swapChain == VK_NULL_HANDLE)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        if (renderFinishedSemaphore ==
            VK_NULL_HANDLE)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        VkQueue presentQueue =
            m_device->GetPresentQueue();


        if (presentQueue == VK_NULL_HANDLE)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }


        VkPresentInfoKHR presentInfo{};

        presentInfo.sType =
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.pNext = nullptr;

        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pWaitSemaphores =
            &renderFinishedSemaphore;

        presentInfo.swapchainCount = 1;

        presentInfo.pSwapchains =
            &m_swapChain;

        presentInfo.pImageIndices =
            &imageIndex;

        presentInfo.pResults = nullptr;


        return vkQueuePresentKHR(
            presentQueue,
            &presentInfo
        );
    }

} // namespace OLIA_ENGINE