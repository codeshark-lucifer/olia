#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdint>

namespace OLIA_ENGINE
{
    class Device;

    // ============================================================
    // Pipeline
    //
    // Owns:
    //     VkRenderPass
    //     VkDescriptorSetLayout
    //     VkPipelineLayout
    //     VkPipeline
    // ============================================================

    class Pipeline
    {
    public:
        Pipeline();
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        Pipeline(Pipeline &&) = delete;
        Pipeline &operator=(Pipeline &&) = delete;

        bool Initialize(
            Device &device,
            VkFormat colorFormat,
            const std::string &vertShaderPath,
            const std::string &fragShaderPath
        );

        void Shutdown();

        // Getters
        VkRenderPass GetRenderPass() const { return m_renderPass; }
        VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
        VkPipeline GetPipeline() const { return m_pipeline; }
        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

        bool IsInitialized() const { return m_pipeline != VK_NULL_HANDLE; }

    private:
        bool CreateRenderPass(VkFormat colorFormat);
        bool CreateDescriptorSetLayout();
        bool CreateGraphicsPipeline(
            const std::string &vertShaderPath,
            const std::string &fragShaderPath
        );

        VkShaderModule CreateShaderModule(const std::vector<char> &code);
        static std::vector<char> ReadFile(const std::string &filePath);

        Device *m_device = nullptr;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
    };

} // namespace OLIA_ENGINE
