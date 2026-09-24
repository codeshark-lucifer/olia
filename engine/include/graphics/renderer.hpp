#pragma once

#include "vk/device.hpp"
#include "vk/swapchain.hpp"
#include "vk/command.hpp"
#include "vk/sync.hpp"
#include "vk/pipeline.hpp"
#include "vk/texture.hpp"
#include "graphics/type.hpp"

class SDL_Window;

namespace OLIA_ENGINE
{
    class Renderer
    {
    public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    private:
        SDL_Window *m_window = nullptr;

        Device m_device;
        SwapChain m_swapChain;
        Command m_command;
        Sync m_sync;
        Pipeline m_pipeline;
        
        Texture m_texture;

        std::vector<VkFramebuffer> m_framebuffers;

        // Uniform buffers (per frame in flight)
        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        std::vector<void *> m_uniformBuffersMapped;

        // Descriptors
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;

        uint32_t m_currentFrame = 0;
        bool m_framebufferResized = false;
        bool m_validationEnabled = false;

    public:
        Renderer(SDL_Window *window);
        ~Renderer();

        void CreateDebugMessenger();
        void DestroyDebugMessenger();

        bool CreateInstance(SDL_Window *window);
        bool CreateSurface(SDL_Window *window);

        // Frame rendering cycle
        bool BeginFrame(uint32_t &imageIndex);
        bool EndFrame(uint32_t imageIndex);
        void DrawFrame();

        void RecreateSwapChain();
        void SetFramebufferResized(bool resized) { m_framebufferResized = resized; }

        // Getters
        Device &GetDevice() { return m_device; }
        const Device &GetDevice() const { return m_device; }

        SwapChain &GetSwapChain() { return m_swapChain; }
        const SwapChain &GetSwapChain() const { return m_swapChain; }

        Command &GetCommand() { return m_command; }
        const Command &GetCommand() const { return m_command; }

        Sync &GetSync() { return m_sync; }
        const Sync &GetSync() const { return m_sync; }

        Pipeline &GetPipeline() { return m_pipeline; }
        const Pipeline &GetPipeline() const { return m_pipeline; }

        uint32_t GetCurrentFrame() const { return m_currentFrame; }

    private:
        bool CreateFramebuffers();
        void DestroyFramebuffers();

        bool CreateUniformBuffers();
        void DestroyUniformBuffers();
        void UpdateUniformBuffer(uint32_t currentFrame);

        bool CreateDescriptorPool();
        bool CreateDescriptorSets();
        void DestroyDescriptorPool();

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory
        );
    };

} // namespace OLIA_ENGINE