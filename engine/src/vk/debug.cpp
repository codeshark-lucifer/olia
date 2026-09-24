#include "vk/debug.hpp"

#include <iostream>

namespace OLIA_ENGINE
{
    // ============================================================
    // Vulkan Debug Callback
    // ============================================================

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        (void)messageType;
        (void)pUserData;

        if (pCallbackData == nullptr ||
            pCallbackData->pMessage == nullptr)
        {
            return VK_FALSE;
        }

        const char *prefix = "[VULKAN]";

        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                prefix = "[VULKAN VERBOSE]";
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                prefix = "[VULKAN INFO]";
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                prefix = "[VULKAN WARNING]";
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                prefix = "[VULKAN ERROR]";
                break;

            default:
                break;
        }

        std::cerr
            << prefix
            << ' '
            << pCallbackData->pMessage
            << '\n';

        return VK_FALSE;
    }

    // ============================================================
    // Create Debug Messenger
    // ============================================================

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        if (instance == VK_NULL_HANDLE)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        if (pCreateInfo == nullptr ||
            pDebugMessenger == nullptr)
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        auto function =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(
                    instance,
                    "vkCreateDebugUtilsMessengerEXT"));

        if (function == nullptr)
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        return function(
            instance,
            pCreateInfo,
            pAllocator,
            pDebugMessenger);
    }

    // ============================================================
    // Destroy Debug Messenger
    // ============================================================

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator)
    {
        if (instance == VK_NULL_HANDLE ||
            debugMessenger == VK_NULL_HANDLE)
        {
            return;
        }

        auto function =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(
                    instance,
                    "vkDestroyDebugUtilsMessengerEXT"));

        if (function == nullptr)
        {
            return;
        }

        function(
            instance,
            debugMessenger,
            pAllocator);
    }

    // ============================================================
    // Populate Debug Messenger Create Info
    // ============================================================

    void PopulateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};

        createInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        createInfo.pNext = nullptr;

        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        createInfo.pfnUserCallback =
            DebugCallback;

        createInfo.pUserData =
            nullptr;
    }

} // namespace OLIA_ENGINE