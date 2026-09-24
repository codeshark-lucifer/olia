#include "vk/device.hpp"
#include "vk/vkcontext.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

namespace OLIA_ENGINE
{
    // ============================================================
    // Required Device Extensions
    // ============================================================

    const std::vector<const char *> Device::DEVICE_EXTENSIONS =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    // ============================================================
    // Constructor
    // ============================================================

    Device::Device()
    {
    }


    // ============================================================
    // Destructor
    // ============================================================

    Device::~Device()
    {
        Shutdown();
    }


    // ============================================================
    // Initialize
    // ============================================================

    bool Device::Initialize()
    {
        if (vk_context.instance == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Device] Cannot initialize device: "
                << "VkInstance is null.\n";

            return false;
        }

        if (vk_context.surface == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Device] Cannot initialize device: "
                << "VkSurfaceKHR is null.\n";

            return false;
        }


        // --------------------------------------------------------
        // Select Physical Device
        // --------------------------------------------------------

        if (!PickPhysicalDevice())
        {
            std::cerr
                << "[Device] Failed to find a suitable physical device.\n";

            return false;
        }


        // --------------------------------------------------------
        // Create Logical Device
        // --------------------------------------------------------

        if (!CreateLogicalDevice())
        {
            std::cerr
                << "[Device] Failed to create logical device.\n";

            return false;
        }


        std::cout
            << "[Device] Vulkan device initialized successfully.\n";

        return true;
    }


    // ============================================================
    // Shutdown
    // ============================================================

    void Device::Shutdown()
    {
        if (m_device != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(m_device);

            vkDestroyDevice(
                m_device,
                nullptr
            );

            m_device = VK_NULL_HANDLE;
        }

        m_physicalDevice = VK_NULL_HANDLE;

        m_graphicsQueue = VK_NULL_HANDLE;
        m_presentQueue = VK_NULL_HANDLE;

        m_queueFamilies = {};
    }


    // ============================================================
    // Pick Physical Device
    // ============================================================

    bool Device::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;

        VkResult result =
            vkEnumeratePhysicalDevices(
                vk_context.instance,
                &deviceCount,
                nullptr
            );

        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Device] Failed to enumerate physical devices. "
                << "VkResult: "
                << result
                << '\n';

            return false;
        }

        if (deviceCount == 0)
        {
            std::cerr
                << "[Device] No Vulkan physical devices found.\n";

            return false;
        }


        std::vector<VkPhysicalDevice> devices(deviceCount);

        result =
            vkEnumeratePhysicalDevices(
                vk_context.instance,
                &deviceCount,
                devices.data()
            );

        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Device] Failed to retrieve physical devices. "
                << "VkResult: "
                << result
                << '\n';

            return false;
        }


        // --------------------------------------------------------
        // Find Best Device
        // --------------------------------------------------------

        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
        int bestScore = -1;

        for (VkPhysicalDevice device : devices)
        {
            VkPhysicalDeviceProperties properties{};

            vkGetPhysicalDeviceProperties(
                device,
                &properties
            );

            std::cout
                << "[Device] Found GPU: "
                << properties.deviceName
                << '\n';

            int score =
                RateDeviceSuitability(device);

            if (score > bestScore)
            {
                bestScore = score;
                bestDevice = device;
            }
        }


        if (bestDevice == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Device] No suitable GPU found.\n";

            return false;
        }


        m_physicalDevice = bestDevice;

        m_queueFamilies =
            FindQueueFamilies(m_physicalDevice);


        // --------------------------------------------------------
        // Print Selected GPU
        // --------------------------------------------------------

        VkPhysicalDeviceProperties properties{};

        vkGetPhysicalDeviceProperties(
            m_physicalDevice,
            &properties
        );

        std::cout
            << "[Device] Selected GPU: "
            << properties.deviceName
            << '\n';

        std::cout
            << "[Device] Graphics Queue Family: "
            << m_queueFamilies.graphicsFamily
            << '\n';

        std::cout
            << "[Device] Present Queue Family: "
            << m_queueFamilies.presentFamily
            << '\n';


        return true;
    }


    // ============================================================
    // Check Device Suitability
    // ============================================================

    bool Device::IsDeviceSuitable(
        VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices =
            FindQueueFamilies(device);

        if (!indices.IsComplete())
        {
            return false;
        }

        if (!CheckDeviceExtensionSupport(device))
        {
            return false;
        }


        // --------------------------------------------------------
        // Device Features
        // --------------------------------------------------------

        VkPhysicalDeviceFeatures supportedFeatures{};

        vkGetPhysicalDeviceFeatures(
            device,
            &supportedFeatures
        );


        // --------------------------------------------------------
        // Required Features
        // --------------------------------------------------------

        if (!supportedFeatures.samplerAnisotropy)
        {
            return false;
        }


        return true;
    }


    // ============================================================
    // Rate Device Suitability
    // ============================================================

    int Device::RateDeviceSuitability(
        VkPhysicalDevice device) const
    {
        if (!IsDeviceSuitable(device))
        {
            return 0;
        }


        VkPhysicalDeviceProperties properties{};

        vkGetPhysicalDeviceProperties(
            device,
            &properties
        );


        int score = 0;


        // --------------------------------------------------------
        // Device Type
        // --------------------------------------------------------

        switch (properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 1000;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 500;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score += 100;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score += 10;
                break;

            default:
                break;
        }


        // --------------------------------------------------------
        // Maximum 2D Image Size
        // --------------------------------------------------------

        score +=
            static_cast<int>(
                properties.limits.maxImageDimension2D / 1024
            );


        return score;
    }


    // ============================================================
    // Find Queue Families
    // ============================================================

    QueueFamilyIndices Device::FindQueueFamilies(
        VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices{};

        if (device == VK_NULL_HANDLE)
        {
            return indices;
        }


        // --------------------------------------------------------
        // Get Queue Family Count
        // --------------------------------------------------------

        uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queueFamilyCount,
            nullptr
        );

        if (queueFamilyCount == 0)
        {
            return indices;
        }


        std::vector<VkQueueFamilyProperties> queueFamilies(
            queueFamilyCount
        );

        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queueFamilyCount,
            queueFamilies.data()
        );


        // --------------------------------------------------------
        // Find Graphics + Present Queues
        // --------------------------------------------------------

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            const VkQueueFamilyProperties &queueFamily =
                queueFamilies[i];


            // ----------------------------------------------------
            // Graphics
            // ----------------------------------------------------

            if (queueFamily.queueCount > 0 &&
                (queueFamily.queueFlags &
                 VK_QUEUE_GRAPHICS_BIT))
            {
                if (!indices.graphicsFound)
                {
                    indices.graphicsFamily = i;
                    indices.graphicsFound = true;
                }
            }


            // ----------------------------------------------------
            // Present
            // ----------------------------------------------------

            if (vk_context.surface != VK_NULL_HANDLE)
            {
                VkBool32 presentSupport = VK_FALSE;

                VkResult result =
                    vkGetPhysicalDeviceSurfaceSupportKHR(
                        device,
                        i,
                        vk_context.surface,
                        &presentSupport
                    );

                if (result == VK_SUCCESS &&
                    presentSupport == VK_TRUE)
                {
                    if (!indices.presentFound)
                    {
                        indices.presentFamily = i;
                        indices.presentFound = true;
                    }
                }
            }


            // ----------------------------------------------------
            // Stop When Complete
            // ----------------------------------------------------

            if (indices.IsComplete())
            {
                break;
            }
        }


        return indices;
    }


    // ============================================================
    // Check Device Extensions
    // ============================================================

    bool Device::CheckDeviceExtensionSupport(
        VkPhysicalDevice device) const
    {
        uint32_t extensionCount = 0;

        VkResult result =
            vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extensionCount,
                nullptr
            );

        if (result != VK_SUCCESS)
        {
            return false;
        }


        std::vector<VkExtensionProperties> availableExtensions(
            extensionCount
        );

        result =
            vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extensionCount,
                availableExtensions.data()
            );

        if (result != VK_SUCCESS)
        {
            return false;
        }


        std::set<std::string> requiredExtensions(
            DEVICE_EXTENSIONS.begin(),
            DEVICE_EXTENSIONS.end()
        );


        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(
                extension.extensionName
            );
        }


        return requiredExtensions.empty();
    }


    // ============================================================
    // Create Logical Device
    // ============================================================

    bool Device::CreateLogicalDevice()
    {
        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Device] Cannot create logical device: "
                << "physical device is null.\n";

            return false;
        }


        if (!m_queueFamilies.IsComplete())
        {
            std::cerr
                << "[Device] Queue families are incomplete.\n";

            return false;
        }


        // --------------------------------------------------------
        // Unique Queue Families
        // --------------------------------------------------------

        std::set<uint32_t> uniqueQueueFamilies =
        {
            m_queueFamilies.graphicsFamily,
            m_queueFamilies.presentFamily
        };


        float queuePriority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo>
            queueCreateInfos;


        for (uint32_t queueFamily :
             uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};

            queueCreateInfo.sType =
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

            queueCreateInfo.pNext = nullptr;

            queueCreateInfo.queueFamilyIndex =
                queueFamily;

            queueCreateInfo.queueCount = 1;

            queueCreateInfo.pQueuePriorities =
                &queuePriority;

            queueCreateInfos.push_back(
                queueCreateInfo
            );
        }


        // --------------------------------------------------------
        // Supported Features
        // --------------------------------------------------------

        VkPhysicalDeviceFeatures supportedFeatures{};

        vkGetPhysicalDeviceFeatures(
            m_physicalDevice,
            &supportedFeatures
        );


        VkPhysicalDeviceFeatures deviceFeatures{};


        // --------------------------------------------------------
        // Enable Features
        // --------------------------------------------------------

        deviceFeatures.samplerAnisotropy =
            VK_TRUE;


        // --------------------------------------------------------
        // Logical Device Create Info
        // --------------------------------------------------------

        VkDeviceCreateInfo createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pNext = nullptr;

        createInfo.queueCreateInfoCount =
            static_cast<uint32_t>(
                queueCreateInfos.size()
            );

        createInfo.pQueueCreateInfos =
            queueCreateInfos.data();

        createInfo.enabledExtensionCount =
            static_cast<uint32_t>(
                DEVICE_EXTENSIONS.size()
            );

        createInfo.ppEnabledExtensionNames =
            DEVICE_EXTENSIONS.data();

        createInfo.pEnabledFeatures =
            &deviceFeatures;


        // --------------------------------------------------------
        // Create Logical Device
        // --------------------------------------------------------

        VkResult result =
            vkCreateDevice(
                m_physicalDevice,
                &createInfo,
                nullptr,
                &m_device
            );

        if (result != VK_SUCCESS)
        {
            std::cerr
                << "[Device] vkCreateDevice failed. "
                << "VkResult: "
                << result
                << '\n';

            m_device = VK_NULL_HANDLE;

            return false;
        }


        // --------------------------------------------------------
        // Get Graphics Queue
        // --------------------------------------------------------

        vkGetDeviceQueue(
            m_device,
            m_queueFamilies.graphicsFamily,
            0,
            &m_graphicsQueue
        );


        // --------------------------------------------------------
        // Get Present Queue
        // --------------------------------------------------------

        vkGetDeviceQueue(
            m_device,
            m_queueFamilies.presentFamily,
            0,
            &m_presentQueue
        );


        if (m_graphicsQueue == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Device] Failed to retrieve graphics queue.\n";

            Shutdown();

            return false;
        }


        if (m_presentQueue == VK_NULL_HANDLE)
        {
            std::cerr
                << "[Device] Failed to retrieve present queue.\n";

            Shutdown();

            return false;
        }


        // --------------------------------------------------------
        // Print Information
        // --------------------------------------------------------

        VkPhysicalDeviceProperties properties{};

        vkGetPhysicalDeviceProperties(
            m_physicalDevice,
            &properties
        );

        std::cout
            << "[Device] Logical device created.\n";

        std::cout
            << "[Device] GPU: "
            << properties.deviceName
            << '\n';

        std::cout
            << "[Device] Graphics Queue: "
            << m_queueFamilies.graphicsFamily
            << '\n';

        std::cout
            << "[Device] Present Queue: "
            << m_queueFamilies.presentFamily
            << '\n';


        return true;
    }


    // ============================================================
    // Get Physical Device Properties
    // ============================================================

    VkPhysicalDeviceProperties Device::GetProperties() const
    {
        VkPhysicalDeviceProperties properties{};

        if (m_physicalDevice != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceProperties(
                m_physicalDevice,
                &properties
            );
        }

        return properties;
    }


    // ============================================================
    // Get Physical Device Features
    // ============================================================

    VkPhysicalDeviceFeatures Device::GetFeatures() const
    {
        VkPhysicalDeviceFeatures features{};

        if (m_physicalDevice != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceFeatures(
                m_physicalDevice,
                &features
            );
        }

        return features;
    }


    // ============================================================
    // Get Physical Device Memory Properties
    // ============================================================

    VkPhysicalDeviceMemoryProperties
    Device::GetMemoryProperties() const
    {
        VkPhysicalDeviceMemoryProperties memoryProperties{};

        if (m_physicalDevice != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceMemoryProperties(
                m_physicalDevice,
                &memoryProperties
            );
        }

        return memoryProperties;
    }

} // namespace OLIA_ENGINE