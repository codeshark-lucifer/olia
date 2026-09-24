#pragma once
#include <vulkan/vulkan.h>
#include <malloc.h>
#include <cstdlib>
#include <iostream>

namespace OLIA_ENGINE
{
    class Allocation
    {
    private:
        VkAllocationCallbacks m_allocatorCallbacks{};

        static void *VKAPI_CALL M_Allocation(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope)
        {
            return _aligned_malloc(size, alignment);
        }

        static void *VKAPI_CALL M_Reallocation(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope)
        {
            if (size == 0)
            {
                if (pOriginal)
                    _aligned_free(pOriginal);
                return nullptr;
            }

            return _aligned_realloc(pOriginal, size, alignment);
        }

        static void VKAPI_CALL M_Free(void *pUserData, void *pMemory)
        {
            if (pMemory)
            {
                _aligned_free(pMemory);
            }
        }

    public:
        Allocation()
        {
            // 2. Populate the Vulkan allocation callbacks structure
            m_allocatorCallbacks.pUserData = this; // Optional context pointer
            m_allocatorCallbacks.pfnAllocation = M_Allocation;
            m_allocatorCallbacks.pfnReallocation = M_Reallocation;
            m_allocatorCallbacks.pfnFree = M_Free;
            m_allocatorCallbacks.pfnInternalAllocation = nullptr;
            m_allocatorCallbacks.pfnInternalFree = nullptr;
        }

        ~Allocation() = default;

        VkAllocationCallbacks *callbacks()
        {
            return &m_allocatorCallbacks;
        }
    };
    extern Allocation allocation;
} // namespace OLIA_ENGINE
