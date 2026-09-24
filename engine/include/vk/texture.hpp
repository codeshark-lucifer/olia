#pragma once

#include <vulkan/vulkan.h>

#include "core/allocation.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <cstring>
#include <stb/stb_image.h>

namespace OLIA_ENGINE
{
    class Texture
    {
    private:
        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;

        VkImageView m_view = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;

        VkFormat m_format = VK_FORMAT_R8G8B8A8_SRGB;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_channels = 0;

        VkImageLayout m_layout =
            VK_IMAGE_LAYOUT_UNDEFINED;

        bool m_initialized = false;

    public:
        Texture() = default;

        ~Texture()
        {
            Destroy();
        }

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&other) noexcept
        {
            MoveFrom(other);
        }

        Texture &operator=(Texture &&other) noexcept
        {
            if (this != &other)
            {
                Destroy();
                MoveFrom(other);
            }

            return *this;
        }

    public:
        // ============================================================
        // Initialize from image file
        // ============================================================

        bool Init(
            VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            const std::string &filename)
        {
            if (m_initialized)
            {
                throw std::runtime_error(
                    "Texture is already initialized.");
            }

            if (device == VK_NULL_HANDLE ||
                physicalDevice == VK_NULL_HANDLE ||
                commandPool == VK_NULL_HANDLE ||
                graphicsQueue == VK_NULL_HANDLE)
            {
                throw std::runtime_error(
                    "Invalid Vulkan texture initialization arguments.");
            }

            m_device = device;
            m_physicalDevice = physicalDevice;

            // --------------------------------------------------------
            // Load image using stb_image
            // --------------------------------------------------------

            int width = 0;
            int height = 0;
            int channels = 0;

            stbi_uc *pixels = stbi_load(
                filename.c_str(),
                &width,
                &height,
                &channels,
                STBI_rgb_alpha);

            if (!pixels)
            {
                throw std::runtime_error(
                    "Failed to load texture: " + filename);
            }

            m_width = static_cast<uint32_t>(width);
            m_height = static_cast<uint32_t>(height);
            m_channels = 4;

            VkDeviceSize imageSize =
                static_cast<VkDeviceSize>(
                    m_width) *
                static_cast<VkDeviceSize>(
                    m_height) *
                4;

            // --------------------------------------------------------
            // Create staging buffer
            // --------------------------------------------------------

            VkBuffer stagingBuffer = VK_NULL_HANDLE;
            VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

            CreateBuffer(
                imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingMemory);

            void *data = nullptr;

            VkResult mapResult = vkMapMemory(
                m_device,
                stagingMemory,
                0,
                imageSize,
                0,
                &data);

            if (mapResult != VK_SUCCESS)
            {
                stbi_image_free(pixels);

                vkDestroyBuffer(
                    m_device,
                    stagingBuffer,
                    allocation.callbacks());

                vkFreeMemory(
                    m_device,
                    stagingMemory,
                    allocation.callbacks());

                throw std::runtime_error(
                    "Failed to map texture staging memory.");
            }

            std::memcpy(
                data,
                pixels,
                static_cast<size_t>(imageSize));

            vkUnmapMemory(
                m_device,
                stagingMemory);

            stbi_image_free(pixels);

            // --------------------------------------------------------
            // Create Vulkan image
            // --------------------------------------------------------

            CreateImage();

            AllocateMemory();

            // --------------------------------------------------------
            // Upload staging buffer into image
            // --------------------------------------------------------

            VkCommandBuffer commandBuffer =
                BeginSingleTimeCommands(commandPool);

            TransitionImageLayout(
                commandBuffer,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            CopyBufferToImage(
                commandBuffer,
                stagingBuffer);

            TransitionImageLayout(
                commandBuffer,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            EndSingleTimeCommands(
                commandPool,
                graphicsQueue,
                commandBuffer);

            // Staging resources are no longer needed.
            vkDestroyBuffer(
                m_device,
                stagingBuffer,
                allocation.callbacks());

            vkFreeMemory(
                m_device,
                stagingMemory,
                allocation.callbacks());

            // --------------------------------------------------------
            // Create view
            // --------------------------------------------------------

            CreateImageView();

            // --------------------------------------------------------
            // Create sampler
            // --------------------------------------------------------

            CreateSampler();

            m_initialized = true;

            return true;
        }

        void Bind(VkCommandBuffer cmd, VkPipelineLayout layout)
        {
            int32_t sampleTexture = 1;

            vkCmdPushConstants(
                cmd,
                layout,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(sampleTexture),
                &sampleTexture);
        }

        // ============================================================
        // Destroy
        // ============================================================

        void Destroy()
        {
            if (m_device == VK_NULL_HANDLE)
                return;

            if (m_sampler != VK_NULL_HANDLE)
            {
                vkDestroySampler(
                    m_device,
                    m_sampler,
                    allocation.callbacks());

                m_sampler = VK_NULL_HANDLE;
            }

            if (m_view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(
                    m_device,
                    m_view,
                    allocation.callbacks());

                m_view = VK_NULL_HANDLE;
            }

            if (m_image != VK_NULL_HANDLE)
            {
                vkDestroyImage(
                    m_device,
                    m_image,
                    allocation.callbacks());

                m_image = VK_NULL_HANDLE;
            }

            if (m_memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(
                    m_device,
                    m_memory,
                    allocation.callbacks());

                m_memory = VK_NULL_HANDLE;
            }

            m_device = VK_NULL_HANDLE;
            m_physicalDevice = VK_NULL_HANDLE;

            m_width = 0;
            m_height = 0;
            m_channels = 0;

            m_layout =
                VK_IMAGE_LAYOUT_UNDEFINED;

            m_initialized = false;
        }

    private:
        // ============================================================
        // Create Image
        // ============================================================

        void CreateImage()
        {
            VkImageCreateInfo imageInfo{};

            imageInfo.sType =
                VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

            imageInfo.imageType =
                VK_IMAGE_TYPE_2D;

            imageInfo.extent.width =
                m_width;

            imageInfo.extent.height =
                m_height;

            imageInfo.extent.depth = 1;

            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;

            imageInfo.format =
                m_format;

            imageInfo.tiling =
                VK_IMAGE_TILING_OPTIMAL;

            imageInfo.initialLayout =
                VK_IMAGE_LAYOUT_UNDEFINED;

            imageInfo.usage =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT;

            imageInfo.sharingMode =
                VK_SHARING_MODE_EXCLUSIVE;

            imageInfo.samples =
                VK_SAMPLE_COUNT_1_BIT;

            imageInfo.flags = 0;

            if (vkCreateImage(
                    m_device,
                    &imageInfo,
                    allocation.callbacks(),
                    &m_image) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to create Vulkan texture image.");
            }
        }

        // ============================================================
        // Allocate Image Memory
        // ============================================================

        void AllocateMemory()
        {
            VkMemoryRequirements requirements{};

            vkGetImageMemoryRequirements(
                m_device,
                m_image,
                &requirements);

            VkPhysicalDeviceMemoryProperties memoryProperties{};

            vkGetPhysicalDeviceMemoryProperties(
                m_physicalDevice,
                &memoryProperties);

            VkMemoryAllocateInfo allocInfo{};

            allocInfo.sType =
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

            allocInfo.allocationSize =
                requirements.size;

            allocInfo.memoryTypeIndex =
                FindMemoryType(
                    memoryProperties,
                    requirements.memoryTypeBits,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(
                    m_device,
                    &allocInfo,
                    allocation.callbacks(),
                    &m_memory) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to allocate texture memory.");
            }

            if (vkBindImageMemory(
                    m_device,
                    m_image,
                    m_memory,
                    0) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to bind texture image memory.");
            }
        }

        // ============================================================
        // Create Image View
        // ============================================================

        void CreateImageView()
        {
            VkImageViewCreateInfo viewInfo{};

            viewInfo.sType =
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            viewInfo.image =
                m_image;

            viewInfo.viewType =
                VK_IMAGE_VIEW_TYPE_2D;

            viewInfo.format =
                m_format;

            viewInfo.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;

            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(
                    m_device,
                    &viewInfo,
                    allocation.callbacks(),
                    &m_view) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to create texture image view.");
            }
        }

        // ============================================================
        // Create Sampler
        // ============================================================

        void CreateSampler()
        {
            VkSamplerCreateInfo samplerInfo{};

            samplerInfo.sType =
                VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

            samplerInfo.magFilter =
                VK_FILTER_LINEAR;

            samplerInfo.minFilter =
                VK_FILTER_LINEAR;

            samplerInfo.addressModeU =
                VK_SAMPLER_ADDRESS_MODE_REPEAT;

            samplerInfo.addressModeV =
                VK_SAMPLER_ADDRESS_MODE_REPEAT;

            samplerInfo.addressModeW =
                VK_SAMPLER_ADDRESS_MODE_REPEAT;

            samplerInfo.anisotropyEnable =
                VK_FALSE;

            samplerInfo.maxAnisotropy =
                1.0f;

            samplerInfo.borderColor =
                VK_BORDER_COLOR_INT_OPAQUE_BLACK;

            samplerInfo.unnormalizedCoordinates =
                VK_FALSE;

            samplerInfo.compareEnable =
                VK_FALSE;

            samplerInfo.compareOp =
                VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode =
                VK_SAMPLER_MIPMAP_MODE_LINEAR;

            samplerInfo.mipLodBias =
                0.0f;

            samplerInfo.minLod =
                0.0f;

            samplerInfo.maxLod =
                0.0f;

            if (vkCreateSampler(
                    m_device,
                    &samplerInfo,
                    allocation.callbacks(),
                    &m_sampler) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to create texture sampler.");
            }
        }

        // ============================================================
        // Create Buffer
        // ============================================================

        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &memory)
        {
            VkBufferCreateInfo bufferInfo{};

            bufferInfo.sType =
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

            bufferInfo.size =
                size;

            bufferInfo.usage =
                usage;

            bufferInfo.sharingMode =
                VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(
                    m_device,
                    &bufferInfo,
                    allocation.callbacks(),
                    &buffer) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to create texture staging buffer.");
            }

            VkMemoryRequirements requirements{};

            vkGetBufferMemoryRequirements(
                m_device,
                buffer,
                &requirements);

            VkPhysicalDeviceMemoryProperties memoryProperties{};

            vkGetPhysicalDeviceMemoryProperties(
                m_physicalDevice,
                &memoryProperties);

            VkMemoryAllocateInfo allocInfo{};

            allocInfo.sType =
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

            allocInfo.allocationSize =
                requirements.size;

            allocInfo.memoryTypeIndex =
                FindMemoryType(
                    memoryProperties,
                    requirements.memoryTypeBits,
                    properties);

            if (vkAllocateMemory(
                    m_device,
                    &allocInfo,
                    allocation.callbacks(),
                    &memory) != VK_SUCCESS)
            {
                vkDestroyBuffer(
                    m_device,
                    buffer,
                    allocation.callbacks());

                buffer = VK_NULL_HANDLE;

                throw std::runtime_error(
                    "Failed to allocate texture staging memory.");
            }

            vkBindBufferMemory(
                m_device,
                buffer,
                memory,
                0);
        }

        // ============================================================
        // Find Memory Type
        // ============================================================

        uint32_t FindMemoryType(
            const VkPhysicalDeviceMemoryProperties &properties,
            uint32_t typeFilter,
            VkMemoryPropertyFlags flags) const
        {
            for (uint32_t i = 0;
                 i < properties.memoryTypeCount;
                 ++i)
            {
                if ((typeFilter & (1u << i)) &&
                    (properties.memoryTypes[i].propertyFlags &
                     flags) == flags)
                {
                    return i;
                }
            }

            throw std::runtime_error(
                "Failed to find suitable texture memory type.");
        }

        // ============================================================
        // Begin Single Time Command
        // ============================================================

        VkCommandBuffer BeginSingleTimeCommands(
            VkCommandPool commandPool)
        {
            VkCommandBufferAllocateInfo allocInfo{};

            allocInfo.sType =
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

            allocInfo.level =
                VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            allocInfo.commandPool =
                commandPool;

            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;

            if (vkAllocateCommandBuffers(
                    m_device,
                    &allocInfo,
                    &commandBuffer) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to allocate texture command buffer.");
            }

            VkCommandBufferBeginInfo beginInfo{};

            beginInfo.sType =
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            beginInfo.flags =
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(
                    commandBuffer,
                    &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to begin texture command buffer.");
            }

            return commandBuffer;
        }

        // ============================================================
        // End Single Time Command
        // ============================================================

        void EndSingleTimeCommands(
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkCommandBuffer commandBuffer)
        {
            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to end texture command buffer.");
            }

            VkSubmitInfo submitInfo{};

            submitInfo.sType =
                VK_STRUCTURE_TYPE_SUBMIT_INFO;

            submitInfo.commandBufferCount = 1;

            submitInfo.pCommandBuffers =
                &commandBuffer;

            if (vkQueueSubmit(
                    graphicsQueue,
                    1,
                    &submitInfo,
                    VK_NULL_HANDLE) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to submit texture command buffer.");
            }

            vkQueueWaitIdle(graphicsQueue);

            vkFreeCommandBuffers(
                m_device,
                commandPool,
                1,
                &commandBuffer);
        }

        // ============================================================
        // Transition Image Layout
        // ============================================================

        void TransitionImageLayout(
            VkCommandBuffer commandBuffer,
            VkImageLayout oldLayout,
            VkImageLayout newLayout)
        {
            VkImageMemoryBarrier barrier{};

            barrier.sType =
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            barrier.oldLayout =
                oldLayout;

            barrier.newLayout =
                newLayout;

            barrier.srcQueueFamilyIndex =
                VK_QUEUE_FAMILY_IGNORED;

            barrier.dstQueueFamilyIndex =
                VK_QUEUE_FAMILY_IGNORED;

            barrier.image =
                m_image;

            barrier.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;

            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (oldLayout ==
                    VK_IMAGE_LAYOUT_UNDEFINED &&
                newLayout ==
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;

                barrier.dstAccessMask =
                    VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage =
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

                destinationStage =
                    VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (
                oldLayout ==
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                newLayout ==
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask =
                    VK_ACCESS_TRANSFER_WRITE_BIT;

                barrier.dstAccessMask =
                    VK_ACCESS_SHADER_READ_BIT;

                sourceStage =
                    VK_PIPELINE_STAGE_TRANSFER_BIT;

                destinationStage =
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else
            {
                throw std::runtime_error(
                    "Unsupported texture image layout transition.");
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage,
                destinationStage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);

            m_layout = newLayout;
        }

        // ============================================================
        // Copy Buffer -> Image
        // ============================================================

        void CopyBufferToImage(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer)
        {
            VkBufferImageCopy region{};

            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            region.imageSubresource.mipLevel = 0;

            region.imageSubresource.baseArrayLayer = 0;

            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};

            region.imageExtent =
                {
                    m_width,
                    m_height,
                    1};

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                m_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region);
        }

        // ============================================================
        // Move
        // ============================================================

        void MoveFrom(Texture &other) noexcept
        {
            m_device =
                other.m_device;

            m_physicalDevice =
                other.m_physicalDevice;

            m_image =
                other.m_image;

            m_memory =
                other.m_memory;

            m_view =
                other.m_view;

            m_sampler =
                other.m_sampler;

            m_format =
                other.m_format;

            m_width =
                other.m_width;

            m_height =
                other.m_height;

            m_channels =
                other.m_channels;

            m_layout =
                other.m_layout;

            m_initialized =
                other.m_initialized;

            other.m_device =
                VK_NULL_HANDLE;

            other.m_physicalDevice =
                VK_NULL_HANDLE;

            other.m_image =
                VK_NULL_HANDLE;

            other.m_memory =
                VK_NULL_HANDLE;

            other.m_view =
                VK_NULL_HANDLE;

            other.m_sampler =
                VK_NULL_HANDLE;

            other.m_initialized =
                false;
        }

    public:
        // ============================================================
        // Getters
        // ============================================================

        VkImage GetImage() const
        {
            return m_image;
        }

        VkImageView GetView() const
        {
            return m_view;
        }

        VkSampler GetSampler() const
        {
            return m_sampler;
        }

        VkFormat GetFormat() const
        {
            return m_format;
        }

        VkImageLayout GetLayout() const
        {
            return m_layout;
        }

        uint32_t GetWidth() const
        {
            return m_width;
        }

        uint32_t GetHeight() const
        {
            return m_height;
        }

        uint32_t GetChannels() const
        {
            return m_channels;
        }

        bool IsInitialized() const
        {
            return m_initialized;
        }
    };

} // namespace OLIA_ENGINE