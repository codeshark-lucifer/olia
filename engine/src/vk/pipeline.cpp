#include "vk/pipeline.hpp"
#include "vk/device.hpp"
#include "graphics/type.hpp"

#include <fstream>
#include <iostream>

namespace OLIA_ENGINE
{
    // ============================================================
    // Constructor / Destructor
    // ============================================================

    Pipeline::Pipeline()
    {
    }

    Pipeline::~Pipeline()
    {
        Shutdown();
    }

    // ============================================================
    // Initialize
    // ============================================================

    bool Pipeline::Initialize(
        Device &device,
        VkFormat colorFormat,
        const std::string &vertShaderPath,
        const std::string &fragShaderPath)
    {
        if (!device.IsInitialized())
        {
            std::cerr << "[Pipeline] Device is not initialized.\n";
            return false;
        }

        m_device = &device;

        if (!CreateRenderPass(colorFormat))
        {
            Shutdown();
            return false;
        }

        if (!CreateDescriptorSetLayout())
        {
            Shutdown();
            return false;
        }

        if (!CreateGraphicsPipeline(vertShaderPath, fragShaderPath))
        {
            Shutdown();
            return false;
        }

        std::cout << "[Pipeline] Graphics pipeline created successfully.\n";
        return true;
    }

    // ============================================================
    // Shutdown
    // ============================================================

    void Pipeline::Shutdown()
    {
        if (m_device == nullptr)
        {
            return;
        }

        VkDevice device = m_device->GetDevice();
        if (device != VK_NULL_HANDLE)
        {
            if (m_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device, m_pipeline, nullptr);
                m_pipeline = VK_NULL_HANDLE;
            }

            if (m_pipelineLayout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
                m_pipelineLayout = VK_NULL_HANDLE;
            }

            if (m_descriptorSetLayout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
                m_descriptorSetLayout = VK_NULL_HANDLE;
            }

            if (m_renderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(device, m_renderPass, nullptr);
                m_renderPass = VK_NULL_HANDLE;
            }
        }

        m_device = nullptr;
    }

    // ============================================================
    // Create Render Pass
    // ============================================================

    bool Pipeline::CreateRenderPass(VkFormat colorFormat)
    {
        VkDevice device = m_device->GetDevice();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = colorFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[Pipeline] Failed to create render pass. VkResult: " << result << '\n';
            return false;
        }

        return true;
    }

    // ============================================================
    // Create Descriptor Set Layout
    // ============================================================

    bool Pipeline::CreateDescriptorSetLayout()
    {
        VkDevice device = m_device->GetDevice();

        // Binding 0: UniformBufferObject (model, view, proj)
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        // Binding 1: renderTexture sampler
        VkDescriptorSetLayoutBinding sampler1LayoutBinding{};
        sampler1LayoutBinding.binding = 1;
        sampler1LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler1LayoutBinding.descriptorCount = 1;
        sampler1LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler1LayoutBinding.pImmutableSamplers = nullptr;

        // Binding 2: specularTexture sampler
        VkDescriptorSetLayoutBinding sampler2LayoutBinding{};
        sampler2LayoutBinding.binding = 2;
        sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler2LayoutBinding.descriptorCount = 1;
        sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler2LayoutBinding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
            uboLayoutBinding,
            sampler1LayoutBinding,
            sampler2LayoutBinding
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[Pipeline] Failed to create descriptor set layout. VkResult: " << result << '\n';
            return false;
        }

        return true;
    }

    // ============================================================
    // Create Graphics Pipeline
    // ============================================================

    bool Pipeline::CreateGraphicsPipeline(
        const std::string &vertShaderPath,
        const std::string &fragShaderPath)
    {
        VkDevice device = m_device->GetDevice();

        // Read Shaders
        std::vector<char> vertShaderCode = ReadFile(vertShaderPath);
        std::vector<char> fragShaderCode = ReadFile(fragShaderPath);

        if (vertShaderCode.empty() || fragShaderCode.empty())
        {
            std::cerr << "[Pipeline] Failed to load shader files.\n";
            return false;
        }

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE)
        {
            if (vertShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, vertShaderModule, nullptr);
            if (fragShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, fragShaderModule, nullptr);
            return false;
        }

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertShaderStageInfo,
            fragShaderStageInfo
        };

        // Vertex Input State
        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Dynamic Viewport and Scissor
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = nullptr; // Handled dynamically
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;   // Handled dynamically

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE; // Cull none so triangle is visible from both sides
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Color Blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // Push Constants (for sampleTexture flag in main.frag)
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(int32_t);

        // Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[Pipeline] Failed to create pipeline layout. VkResult: " << result << '\n';
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            return false;
        }

        // Create Graphics Pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;

        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);

        if (result != VK_SUCCESS)
        {
            std::cerr << "[Pipeline] Failed to create graphics pipeline. VkResult: " << result << '\n';
            return false;
        }

        return true;
    }

    // ============================================================
    // Create Shader Module
    // ============================================================

    VkShaderModule Pipeline::CreateShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VkResult result = vkCreateShaderModule(m_device->GetDevice(), &createInfo, nullptr, &shaderModule);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[Pipeline] Failed to create shader module. VkResult: " << result << '\n';
            return VK_NULL_HANDLE;
        }

        return shaderModule;
    }

    // ============================================================
    // Read File
    // ============================================================

    std::vector<char> Pipeline::ReadFile(const std::string &filePath)
    {
        // Try direct path first, then fallback paths
        const std::vector<std::string> searchPaths = {
            filePath,
            "../" + filePath,
            "../../" + filePath,
            "assets/" + filePath,
            "../assets/" + filePath,
            "../../assets/" + filePath,
            "assets/shaders/compiled/" + filePath,
            "../assets/shaders/compiled/" + filePath,
            "../../assets/shaders/compiled/" + filePath
        };

        for (const std::string &path : searchPaths)
        {
            std::ifstream file(path, std::ios::ate | std::ios::binary);
            if (file.is_open())
            {
                size_t fileSize = static_cast<size_t>(file.tellg());
                std::vector<char> buffer(fileSize);
                file.seekg(0);
                file.read(buffer.data(), fileSize);
                file.close();
                return buffer;
            }
        }

        std::cerr << "[Pipeline] Failed to open shader file: " << filePath << '\n';
        return {};
    }

} // namespace OLIA_ENGINE
