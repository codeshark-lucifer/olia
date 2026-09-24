#pragma once
#include <vulkan/vulkan.h>

namespace OLIA_ENGINE
{
    struct VkContext
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT m_debugMessenger =
            VK_NULL_HANDLE;
    };

    extern VkContext vk_context;

} // namespace OLIA_ENGINE