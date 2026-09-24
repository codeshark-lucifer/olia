#pragma once

#include <vulkan/vulkan.h>

namespace OLIA_ENGINE
{
    // ============================================================
    // Validation Layers
    // ============================================================

#if defined(OLIA_ENABLE_VALIDATION)
    inline constexpr bool ENABLE_VALIDATION_LAYERS = true;
#else
    inline constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

    // ============================================================
    // Debug Callback
    // ============================================================

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    // ============================================================
    // Debug Messenger
    // ============================================================

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator);

    void PopulateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo);

} // namespace OLIA_ENGINE