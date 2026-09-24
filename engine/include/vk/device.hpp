#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace OLIA_ENGINE
{
    // ============================================================
    // Queue Family Indices
    // ============================================================

    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily  = UINT32_MAX;

        bool graphicsFound = false;
        bool presentFound  = false;

        bool IsComplete() const
        {
            return graphicsFound && presentFound;
        }
    };


    // ============================================================
    // Device
    // ============================================================

    class Device
    {
    public:

        Device();
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;


        // ========================================================
        // Initialization
        // ========================================================

        bool Initialize();

        void Shutdown();


        // ========================================================
        // Physical Device
        // ========================================================

        bool PickPhysicalDevice();

        bool IsDeviceSuitable(VkPhysicalDevice device) const;

        int RateDeviceSuitability(VkPhysicalDevice device) const;


        // ========================================================
        // Logical Device
        // ========================================================

        bool CreateLogicalDevice();


        // ========================================================
        // Queue Families
        // ========================================================

        QueueFamilyIndices FindQueueFamilies(
            VkPhysicalDevice device) const;


        // ========================================================
        // Device Extensions
        // ========================================================

        bool CheckDeviceExtensionSupport(
            VkPhysicalDevice device) const;


        // ========================================================
        // Device Information
        // ========================================================

        VkPhysicalDeviceProperties GetProperties() const;

        VkPhysicalDeviceFeatures GetFeatures() const;

        VkPhysicalDeviceMemoryProperties GetMemoryProperties() const;


        // ========================================================
        // Getters
        // ========================================================

        VkPhysicalDevice GetPhysicalDevice() const
        {
            return m_physicalDevice;
        }

        VkDevice GetDevice() const
        {
            return m_device;
        }

        VkQueue GetGraphicsQueue() const
        {
            return m_graphicsQueue;
        }

        VkQueue GetPresentQueue() const
        {
            return m_presentQueue;
        }

        const QueueFamilyIndices &GetQueueFamilies() const
        {
            return m_queueFamilies;
        }

        uint32_t GetGraphicsQueueFamily() const
        {
            return m_queueFamilies.graphicsFamily;
        }

        uint32_t GetPresentQueueFamily() const
        {
            return m_queueFamilies.presentFamily;
        }


        // ========================================================
        // Utility
        // ========================================================

        bool IsInitialized() const
        {
            return m_device != VK_NULL_HANDLE;
        }

    private:

        // ========================================================
        // Vulkan Handles
        // ========================================================

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

        VkDevice m_device = VK_NULL_HANDLE;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;


        // ========================================================
        // Queue Information
        // ========================================================

        QueueFamilyIndices m_queueFamilies{};


        // ========================================================
        // Required Device Extensions
        // ========================================================

        static const std::vector<const char *> DEVICE_EXTENSIONS;
    };

} // namespace OLIA_ENGINE