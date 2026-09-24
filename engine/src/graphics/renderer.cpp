#include "graphics/renderer.hpp"

#include "core/allocation.hpp"
#include "core/built-in.hpp"
#include "ecs/world.h"
#include "vk/vkcontext.hpp"
#include "vk/debug.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <array>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace OLIA_ENGINE
{
    Allocation allocation;
    VkContext vk_context;

    static bool CheckValidationLayerSupport(const std::vector<const char *> &requiredLayers)
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        if (layerCount == 0)
        {
            return false;
        }

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : requiredLayers)
        {
            bool found = false;
            for (const auto &layerProperties : availableLayers)
            {
                if (std::strcmp(layerName, layerProperties.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                return false;
            }
        }

        return true;
    }

    // Constructor

    Renderer::Renderer(SDL_Window *window)
        : m_window(window)
    {
        if (!CreateInstance(window))
        {
            throw std::runtime_error("Failed to create Vulkan instance.");
        }

        CreateDebugMessenger();

        if (!CreateSurface(window))
        {
            throw std::runtime_error("Failed to create Vulkan surface.");
        }

        if (!m_device.Initialize())
        {
            throw std::runtime_error("Failed to initialize Vulkan device.");
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        if (!m_swapChain.Initialize(m_device, static_cast<uint32_t>(w), static_cast<uint32_t>(h)))
        {
            throw std::runtime_error("Failed to initialize swapchain.");
        }

        if (!m_command.Initialize(m_device, MAX_FRAMES_IN_FLIGHT))
        {
            throw std::runtime_error("Failed to initialize command pool and command buffers.");
        }

        if (!m_sync.Initialize(m_device, MAX_FRAMES_IN_FLIGHT, m_swapChain.GetImageCount()))
        {
            throw std::runtime_error("Failed to initialize synchronization objects.");
        }

        if (!m_pipeline.Initialize(
                m_device,
                m_swapChain.GetImageFormat(),
                "assets/shaders/compiled/main.vert.spv",
                "assets/shaders/compiled/main.frag.spv"))
        {
            throw std::runtime_error("Failed to initialize graphics pipeline.");
        }

        if (!CreateFramebuffers())
        {
            throw std::runtime_error("Failed to create framebuffers.");
        }

        if (!CreateUniformBuffers())
        {
            throw std::runtime_error("Failed to create uniform buffers.");
        }

        {
            // create texture
            if (!m_texture.Init(
                    m_device.GetDevice(),
                    m_device.GetPhysicalDevice(),
                    m_command.GetCommandPool(),
                    m_device.GetGraphicsQueue(),
                    "assets/textures/profile.png"))
            {
                throw std::runtime_error("Failed to load profile texture.");
            }
        }

        if (!CreateDescriptorPool() || !CreateDescriptorSets())
        {
            throw std::runtime_error("Failed to create descriptor sets.");
        }

        std::cout << "Vulkan Renderer initialized successfully.\n";
    }

    // Destructor

    Renderer::~Renderer()
    {
        if (m_device.IsInitialized())
        {
            vkDeviceWaitIdle(m_device.GetDevice());
        }

        DestroyDescriptorPool();
        {
            // destroy texture
            m_texture.Destroy();
        }
        DestroyUniformBuffers();

        auto renderers = g_world->query<MeshRenderer>();
        for (auto &entity : renderers)
        {
            MeshFilter *filter = g_world->get_component<MeshFilter>(entity);
            if (!filter)
                continue;

            MeshRenderer *renderer = g_world->get_component<MeshRenderer>(entity);
            renderer->Cleanup(m_device.GetDevice());
        }

        DestroyFramebuffers();

        m_pipeline.Shutdown();
        m_sync.Shutdown();
        m_command.Shutdown();
        m_swapChain.Shutdown();
        m_device.Shutdown();

        DestroyDebugMessenger();

        if (vk_context.surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(vk_context.instance, vk_context.surface, nullptr);
            vk_context.surface = VK_NULL_HANDLE;
        }

        if (vk_context.instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(vk_context.instance, nullptr);
            vk_context.instance = VK_NULL_HANDLE;
        }
    }

    // Create Instance

    bool Renderer::CreateInstance(SDL_Window *window)
    {
        (void)window;

#if defined(_WIN32)
        char envBuf[1024];
        DWORD len = GetEnvironmentVariableA("VK_LAYER_PATH", envBuf, sizeof(envBuf));
        if (len == 0)
        {
            len = GetEnvironmentVariableA("VK_ADD_LAYER_PATH", envBuf, sizeof(envBuf));
        }
        if (len == 0)
        {
            SetEnvironmentVariableA("VK_ADD_LAYER_PATH", "vendor/vulkan/bin;../vendor/vulkan/bin;../../vendor/vulkan/bin;.");
        }
#endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Olia Editor";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Olia Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        Uint32 extensionCount = 0;
        const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        if (extensions == nullptr)
        {
            std::cerr << "Failed to get SDL Vulkan extensions: " << SDL_GetError() << '\n';
            return false;
        }

        std::vector<const char *> extensionNames(extensions, extensions + extensionCount);

        std::vector<const char *> layers;
        m_validationEnabled = false;

        if (ENABLE_VALIDATION_LAYERS)
        {
            std::vector<const char *> requiredLayers = {"VK_LAYER_KHRONOS_validation"};

            if (CheckValidationLayerSupport(requiredLayers))
            {
                m_validationEnabled = true;
                layers.push_back("VK_LAYER_KHRONOS_validation");
                extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else
            {
                std::cerr << "[Renderer] Validation layer VK_LAYER_KHRONOS_validation not found. Continuing without validation layers.\n";
            }
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_validationEnabled)
        {
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
        }

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
        createInfo.ppEnabledExtensionNames = extensionNames.data();
        createInfo.pNext = m_validationEnabled ? &debugCreateInfo : nullptr;

        VkResult result = vkCreateInstance(&createInfo, allocation.callbacks(), &vk_context.instance);
        if (result != VK_SUCCESS)
        {
            std::cerr << "Failed to create Vulkan instance. VkResult: " << result << '\n';
            return false;
        }

        std::cout << "Vulkan instance created successfully.\n";
        return true;
    }

    // Debug Messenger

    void Renderer::CreateDebugMessenger()
    {
        if (!m_validationEnabled)
        {
            std::cout << "Vulkan validation layers disabled.\n";
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        VkResult result = CreateDebugUtilsMessengerEXT(
            vk_context.instance,
            &createInfo,
            allocation.callbacks(),
            &vk_context.m_debugMessenger);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Vulkan debug messenger.");
        }

        std::cout << "Vulkan debug messenger created successfully.\n";
    }

    void Renderer::DestroyDebugMessenger()
    {
        if (!m_validationEnabled)
        {
            return;
        }

        if (vk_context.m_debugMessenger != VK_NULL_HANDLE)
        {
            DestroyDebugUtilsMessengerEXT(
                vk_context.instance,
                vk_context.m_debugMessenger,
                allocation.callbacks());

            vk_context.m_debugMessenger = VK_NULL_HANDLE;
            std::cout << "Vulkan debug messenger destroyed.\n";
        }
    }

    // Create Surface

    bool Renderer::CreateSurface(SDL_Window *window)
    {
        if (window == nullptr || vk_context.instance == VK_NULL_HANDLE)
        {
            return false;
        }

        if (!SDL_Vulkan_CreateSurface(window, vk_context.instance, allocation.callbacks(), &vk_context.surface))
        {
            std::cerr << "Failed to create Vulkan surface: " << SDL_GetError() << '\n';
            return false;
        }

        std::cout << "Vulkan surface created successfully.\n";
        return true;
    }

    // Frame Rendering Lifecycle

    bool Renderer::BeginFrame(uint32_t &imageIndex)
    {
        VkResult waitResult = m_sync.WaitForFrame(m_currentFrame);
        if (waitResult != VK_SUCCESS)
        {
            std::cerr << "[Renderer] Failed to wait for fence.\n";
            return false;
        }

        VkResult acquireResult = m_swapChain.AcquireNextImage(
            m_sync.GetImageAvailable(m_currentFrame),
            &imageIndex);

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return false;
        }
        else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            std::cerr << "[Renderer] Failed to acquire swapchain image. VkResult: " << acquireResult << '\n';
            return false;
        }

        m_sync.ResetFrameFence(m_currentFrame);

        m_command.Reset(m_currentFrame);
        if (!m_command.Begin(m_currentFrame))
        {
            std::cerr << "[Renderer] Failed to begin command buffer.\n";
            return false;
        }

        if (g_world != nullptr)
        {
            auto renderers =
                g_world->query<MeshRenderer>();

            for (ECS::Entity entity : renderers)
            {
                MeshRenderer *meshRenderer =
                    g_world->get_component<MeshRenderer>(entity);

                MeshFilter *meshFilter =
                    g_world->get_component<MeshFilter>(entity);

                if (meshRenderer == nullptr ||
                    meshFilter == nullptr)
                {
                    continue;
                }

                if (!meshFilter->mesh)
                {
                    continue;
                }

                if (meshRenderer->IsInitialized())
                {
                    continue;
                }

                meshRenderer->Init(
                    m_device.GetDevice(),
                    m_device.GetPhysicalDevice(),
                    *meshFilter->mesh);
            }
        }
        return true;
    }

    bool Renderer::EndFrame(uint32_t imageIndex)
    {
        if (!m_command.End(m_currentFrame))
        {
            std::cerr << "[Renderer] Failed to end command buffer.\n";
            return false;
        }

        VkCommandBuffer commandBuffer = m_command.GetCommandBuffer(m_currentFrame);
        VkSemaphore waitSemaphore = m_sync.GetImageAvailable(m_currentFrame);
        VkSemaphore signalSemaphore = m_sync.GetRenderFinished(imageIndex);
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;

        VkResult submitResult = vkQueueSubmit(
            m_device.GetGraphicsQueue(),
            1,
            &submitInfo,
            m_sync.GetInFlightFence(m_currentFrame));

        if (submitResult != VK_SUCCESS)
        {
            std::cerr << "[Renderer] Failed to submit command buffer. VkResult: " << submitResult << '\n';
            return false;
        }

        VkResult presentResult = m_swapChain.Present(
            imageIndex,
            signalSemaphore);

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            RecreateSwapChain();
        }
        else if (presentResult != VK_SUCCESS)
        {
            std::cerr << "[Renderer] Failed to present swapchain image. VkResult: " << presentResult << '\n';
            return false;
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return true;
    }

    void Renderer::DrawFrame()
    {
        uint32_t imageIndex = 0;

        if (!BeginFrame(imageIndex))
            return;

        VkCommandBuffer cmd = m_command.GetCommandBuffer(m_currentFrame);

        // Find camera BEFORE beginning the render pass

        auto cameras = g_world->query<Camera>();

        if (cameras.empty())
        {
            std::cerr << "[Renderer] No camera found.\n";

            m_command.End(m_currentFrame);
            return;
        }

        ECS::Entity camEntity = cameras[0];

        Camera *cam = g_world->get_component<Camera>(camEntity);
        Transform *camTransform = g_world->get_component<Transform>(camEntity);

        if (!cam || !camTransform)
        {
            std::cerr << "[Renderer] Camera is missing Transform or Camera component.\n";

            m_command.End(m_currentFrame);
            return;
        }

        // Begin Render Pass

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_pipeline.GetRenderPass();
        renderPassInfo.framebuffer = m_framebuffers[imageIndex];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain.GetExtent();

        VkClearValue clearColor = {
            {{0.07f, 0.07f, 0.12f, 1.0f}}};

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(
            cmd,
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE);

        // Pipeline

        vkCmdBindPipeline(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline.GetPipeline());

        // Viewport

        VkViewport viewport{};

        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width =
            static_cast<float>(m_swapChain.GetExtent().width);

        viewport.height =
            static_cast<float>(m_swapChain.GetExtent().height);

        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(
            cmd,
            0,
            1,
            &viewport);

        // Scissor

        VkRect2D scissor{};

        scissor.offset = {0, 0};
        scissor.extent = m_swapChain.GetExtent();

        vkCmdSetScissor(
            cmd,
            0,
            1,
            &scissor);

        // Descriptor Set

        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline.GetPipelineLayout(),
            0,
            1,
            &m_descriptorSets[m_currentFrame],
            0,
            nullptr);

        // Push Constant

        m_texture.Bind(
            cmd,
            m_pipeline.GetPipelineLayout());
        // Render Meshes

        auto renderers = g_world->query<MeshRenderer>();

        for (auto &entity : renderers)
        {
            MeshFilter *filter =
                g_world->get_component<MeshFilter>(entity);

            Transform *transform =
                g_world->get_component<Transform>(entity);

            // IMPORTANT:
            // Check for missing components.
            if (!filter || !filter->mesh || !transform)
                continue;

            MeshRenderer *renderer =
                g_world->get_component<MeshRenderer>(entity);

            if (!renderer)
                continue;

            // UBO

            UniformBufferObject ubo{};

            ubo.model = transform->matrix();

            ubo.view =
                cam->viewMatrix(*camTransform);

            ubo.proj =
                cam->projectionMatrix();

            std::memcpy(
                m_uniformBuffersMapped[m_currentFrame],
                &ubo,
                sizeof(UniformBufferObject));

            // Draw

            renderer->Draw(cmd);
        }

        // End Render Pass

        vkCmdEndRenderPass(cmd);

        // Present

        EndFrame(imageIndex);
    }

    void Renderer::RecreateSwapChain()
    {
        if (m_window == nullptr)
        {
            return;
        }

        int width = 0, height = 0;
        SDL_GetWindowSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            SDL_GetWindowSize(m_window, &width, &height);
            SDL_Event event;
            SDL_WaitEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
            {
                return;
            }
        }

        vkDeviceWaitIdle(m_device.GetDevice());

        DestroyFramebuffers();

        m_swapChain.Recreate(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        m_sync.RecreateRenderFinishedSemaphores(m_swapChain.GetImageCount());

        CreateFramebuffers();
    }

    // Framebuffers

    bool Renderer::CreateFramebuffers()
    {
        VkDevice device = m_device.GetDevice();
        uint32_t imageCount = m_swapChain.GetImageCount();
        m_framebuffers.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageView attachments[] = {
                m_swapChain.GetImageView(i)};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_pipeline.GetRenderPass();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapChain.GetExtent().width;
            framebufferInfo.height = m_swapChain.GetExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            {
                std::cerr << "[Renderer] Failed to create framebuffer " << i << '\n';
                return false;
            }
        }

        std::cout << "[Renderer] Framebuffers created: " << imageCount << '\n';
        return true;
    }

    void Renderer::DestroyFramebuffers()
    {
        VkDevice device = m_device.GetDevice();
        for (VkFramebuffer framebuffer : m_framebuffers)
        {
            if (framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            }
        }
        m_framebuffers.clear();
    }

    // Buffer & Memory Helpers

    uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties = m_device.GetMemoryProperties();
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    void Renderer::CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory)
    {
        VkDevice device = m_device.GetDevice();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    // Uniform Buffers

    bool Renderer::CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            CreateBuffer(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_uniformBuffers[i],
                m_uniformBuffersMemory[i]);

            vkMapMemory(m_device.GetDevice(), m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
        }

        std::cout << "[Renderer] Uniform buffers created.\n";
        return true;
    }

    void Renderer::DestroyUniformBuffers()
    {
        VkDevice device = m_device.GetDevice();
        for (size_t i = 0; i < m_uniformBuffers.size(); ++i)
        {
            if (m_uniformBuffersMapped[i] != nullptr)
            {
                vkUnmapMemory(device, m_uniformBuffersMemory[i]);
                m_uniformBuffersMapped[i] = nullptr;
            }

            if (m_uniformBuffers[i] != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
                m_uniformBuffers[i] = VK_NULL_HANDLE;
            }

            if (m_uniformBuffersMemory[i] != VK_NULL_HANDLE)
            {
                vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
                m_uniformBuffersMemory[i] = VK_NULL_HANDLE;
            }
        }

        m_uniformBuffers.clear();
        m_uniformBuffersMemory.clear();
        m_uniformBuffersMapped.clear();
    }

    // Descriptors

    bool Renderer::CreateDescriptorPool()
    {
        VkDevice device = m_device.GetDevice();

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
        {
            std::cerr << "[Renderer] Failed to create descriptor pool.\n";
            return false;
        }

        return true;
    }

    bool Renderer::CreateDescriptorSets()
    {
        VkDevice device = m_device.GetDevice();

        // One descriptor set per frame in flight.
        std::vector<VkDescriptorSetLayout> layouts(
            MAX_FRAMES_IN_FLIGHT,
            m_pipeline.GetDescriptorSetLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateDescriptorSets(
                device,
                &allocInfo,
                m_descriptorSets.data()) != VK_SUCCESS)
        {
            std::cerr << "[Renderer] Failed to allocate descriptor sets.\n";
            return false;
        }

        // ------------------------------------------------------------
        // Texture descriptor
        // ------------------------------------------------------------

        VkDescriptorImageInfo textureInfo{};
        textureInfo.imageLayout = m_texture.GetLayout();
        textureInfo.imageView = m_texture.GetView();
        textureInfo.sampler = m_texture.GetSampler();

        // ------------------------------------------------------------
        // Update every frame's descriptor set
        // ------------------------------------------------------------

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

            // --------------------------------------------------------
            // Binding 0: Uniform Buffer
            // --------------------------------------------------------

            descriptorWrites[0].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            descriptorWrites[0].dstSet =
                m_descriptorSets[i];

            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;

            descriptorWrites[0].descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            descriptorWrites[0].descriptorCount = 1;

            descriptorWrites[0].pBufferInfo =
                &bufferInfo;

            // --------------------------------------------------------
            // Binding 1: Texture
            // --------------------------------------------------------

            descriptorWrites[1].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            descriptorWrites[1].dstSet =
                m_descriptorSets[i];

            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;

            descriptorWrites[1].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            descriptorWrites[1].descriptorCount = 1;

            descriptorWrites[1].pImageInfo =
                &textureInfo;

            // --------------------------------------------------------
            // Binding 2: Same Texture
            // --------------------------------------------------------

            descriptorWrites[2].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            descriptorWrites[2].dstSet =
                m_descriptorSets[i];

            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;

            descriptorWrites[2].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            descriptorWrites[2].descriptorCount = 1;

            descriptorWrites[2].pImageInfo =
                &textureInfo;

            // --------------------------------------------------------
            // Write descriptors
            // --------------------------------------------------------

            vkUpdateDescriptorSets(
                device,
                static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(),
                0,
                nullptr);
        }

        std::cout << "[Renderer] Descriptor sets updated.\n";

        return true;
    }

    void Renderer::DestroyDescriptorPool()
    {
        VkDevice device = m_device.GetDevice();
        if (m_descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
            m_descriptorPool = VK_NULL_HANDLE;
        }
        m_descriptorSets.clear();
    }

} // namespace OLIA_ENGINE