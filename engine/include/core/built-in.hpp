#pragma once

#include "graphics/type.hpp"
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <cstring>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct Transform
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion (w, x, y, z)
    glm::vec3 scale = glm::vec3(1.0f);

    // --- Direction Vectors (Assuming standard OpenGL -Z forward convention) ---

    glm::vec3 forward() const
    {
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 backward() const
    {
        return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::vec3 right() const
    {
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 left() const
    {
        return rotation * glm::vec3(-1.0f, 0.0f, 0.0f);
    }

    glm::vec3 up() const
    {
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 down() const
    {
        return rotation * glm::vec3(0.0f, -1.0f, 0.0f);
    }

    // --- Model Matrix ---

    glm::mat4 matrix() const
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rot = glm::mat4_cast(rotation);
        glm::mat4 scl = glm::scale(glm::mat4(1.0f), scale);

        // Standard TRS Order: Translation * Rotation * Scale
        return trans * rot * scl;
    }
};

class Camera
{
public:
    float fov = 45.0f; // Field of view in degrees
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    bool isOrthographic = false;

    // Orthographic properties (if used)
    float orthoSize = 5.0f;

    // Generates the projection matrix
    glm::mat4 projectionMatrix() const
    {
        if (isOrthographic)
        {
            float orthoLeft = -orthoSize * aspectRatio * 0.5f;
            float orthoRight = orthoSize * aspectRatio * 0.5f;
            float orthoBottom = -orthoSize * 0.5f;
            float orthoTop = orthoSize * 0.5f;
            return glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, nearPlane, farPlane);
        }
        else
        {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }
    }

    // Generates the view matrix using a Transform component
    glm::mat4 viewMatrix(const Transform &transform) const
    {
        // LookAt using the transform's position and forward direction
        glm::vec3 center = transform.position + transform.forward();
        return glm::lookAt(transform.position, center, transform.up());
    }
};

class MeshFilter
{
public:
    std::shared_ptr<Mesh> mesh;

    MeshFilter() = default;
    explicit MeshFilter(std::shared_ptr<Mesh> m) : mesh(std::move(m)) {}
};

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct Uniform
{
    std::vector<Buffer> buffers;
    std::vector<void *> mapped; // uniform mapped
};

class MeshRenderer
{
private:
    Buffer m_vertex{};
    Buffer m_index{};

    uint32_t m_indexCount = 0;
    bool m_initialized = false;

private:
    uint32_t findMemoryType(
        VkPhysicalDevice physicalDevice,
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties{};

        vkGetPhysicalDeviceMemoryProperties(
            physicalDevice,
            &memProperties
        );

        for (uint32_t i = 0;
            i < memProperties.memoryTypeCount;
            ++i)
        {
            if ((typeFilter & (1u << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error(
            "MeshRenderer: failed to find suitable memory type!"
        );
    }

    void createBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        Buffer& outBuffer)
    {
        if (size == 0)
        {
            throw std::runtime_error(
                "MeshRenderer: cannot create zero-sized buffer!"
            );
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType =
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode =
            VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(
                device,
                &bufferInfo,
                nullptr,
                &outBuffer.buffer) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "MeshRenderer: failed to create buffer!"
            );
        }

        VkMemoryRequirements memRequirements{};

        vkGetBufferMemoryRequirements(
            device,
            outBuffer.buffer,
            &memRequirements
        );

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType =
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

        allocInfo.allocationSize =
            memRequirements.size;

        allocInfo.memoryTypeIndex =
            findMemoryType(
                physicalDevice,
                memRequirements.memoryTypeBits,
                properties
            );

        if (vkAllocateMemory(
                device,
                &allocInfo,
                nullptr,
                &outBuffer.memory) != VK_SUCCESS)
        {
            vkDestroyBuffer(
                device,
                outBuffer.buffer,
                nullptr
            );

            outBuffer.buffer = VK_NULL_HANDLE;

            throw std::runtime_error(
                "MeshRenderer: failed to allocate buffer memory!"
            );
        }

        if (vkBindBufferMemory(
                device,
                outBuffer.buffer,
                outBuffer.memory,
                0) != VK_SUCCESS)
        {
            vkFreeMemory(
                device,
                outBuffer.memory,
                nullptr
            );

            vkDestroyBuffer(
                device,
                outBuffer.buffer,
                nullptr
            );

            outBuffer.memory = VK_NULL_HANDLE;
            outBuffer.buffer = VK_NULL_HANDLE;

            throw std::runtime_error(
                "MeshRenderer: failed to bind buffer memory!"
            );
        }
    }

    void uploadBuffer(
        VkDevice device,
        const Buffer& buffer,
        const void* data,
        VkDeviceSize size)
    {
        void* mappedData = nullptr;

        if (vkMapMemory(
                device,
                buffer.memory,
                0,
                size,
                0,
                &mappedData) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "MeshRenderer: failed to map buffer memory!"
            );
        }

        std::memcpy(
            mappedData,
            data,
            static_cast<size_t>(size)
        );

        vkUnmapMemory(
            device,
            buffer.memory
        );
    }

public:

    MeshRenderer() = default;

    ~MeshRenderer() = default;

    // ---------------------------------------------------------
    // Copying is NOT allowed.
    //
    // Vulkan handles have ownership, so copying this component
    // would cause two MeshRenderers to own the same VkBuffer.
    // ---------------------------------------------------------

    MeshRenderer(const MeshRenderer&) = delete;
    MeshRenderer& operator=(const MeshRenderer&) = delete;

    // ---------------------------------------------------------
    // Moving IS allowed.
    //
    // ECS requires this.
    // ---------------------------------------------------------

    MeshRenderer(MeshRenderer&& other) noexcept
    {
        m_vertex = other.m_vertex;
        m_index = other.m_index;

        m_indexCount = other.m_indexCount;
        m_initialized = other.m_initialized;

        // Give ownership to this object.
        other.m_vertex = {};
        other.m_index = {};
        other.m_indexCount = 0;
        other.m_initialized = false;
    }

    MeshRenderer& operator=(MeshRenderer&& other) noexcept
    {
        if (this == &other)
            return *this;

        /*
         * IMPORTANT:
         *
         * We cannot safely destroy our existing Vulkan buffers here
         * because this move operator doesn't receive VkDevice.
         *
         * Normally ECS should move components before they are
         * initialized, which is the case for your MeshRenderer.
         */

        m_vertex = other.m_vertex;
        m_index = other.m_index;

        m_indexCount = other.m_indexCount;
        m_initialized = other.m_initialized;

        other.m_vertex = {};
        other.m_index = {};
        other.m_indexCount = 0;
        other.m_initialized = false;

        return *this;
    }

    // ---------------------------------------------------------
    // State
    // ---------------------------------------------------------

    bool IsInitialized() const
    {
        return m_initialized;
    }

    uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }

    // ---------------------------------------------------------
    // Initialize GPU resources
    // ---------------------------------------------------------

    void Init(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        const Mesh& mesh)
    {
        // Already initialized.
        if (m_initialized)
            return;

        if (mesh.vertices.empty())
        {
            throw std::runtime_error(
                "MeshRenderer: mesh has no vertices!"
            );
        }

        if (mesh.indices.empty())
        {
            throw std::runtime_error(
                "MeshRenderer: mesh has no indices!"
            );
        }

        m_indexCount =
            static_cast<uint32_t>(
                mesh.indices.size()
            );

        const VkDeviceSize vertexSize =
            sizeof(Vertex) *
            mesh.vertices.size();

        const VkDeviceSize indexSize =
            sizeof(uint16_t) *
            mesh.indices.size();

        // -----------------------------------------------------
        // Vertex buffer
        // -----------------------------------------------------

        createBuffer(
            device,
            physicalDevice,
            vertexSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_vertex
        );

        uploadBuffer(
            device,
            m_vertex,
            mesh.vertices.data(),
            vertexSize
        );

        // -----------------------------------------------------
        // Index buffer
        // -----------------------------------------------------

        try
        {
            createBuffer(
                device,
                physicalDevice,
                indexSize,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_index
            );

            uploadBuffer(
                device,
                m_index,
                mesh.indices.data(),
                indexSize
            );
        }
        catch (...)
        {
            // Clean vertex buffer if index creation fails.

            if (m_vertex.buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(
                    device,
                    m_vertex.buffer,
                    nullptr
                );
            }

            if (m_vertex.memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(
                    device,
                    m_vertex.memory,
                    nullptr
                );
            }

            m_vertex = {};

            throw;
        }

        m_initialized = true;
    }

    // ---------------------------------------------------------
    // Cleanup GPU resources
    // ---------------------------------------------------------

    void Cleanup(VkDevice device)
    {
        if (device == VK_NULL_HANDLE)
            return;

        if (m_vertex.buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(
                device,
                m_vertex.buffer,
                nullptr
            );
        }

        if (m_vertex.memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(
                device,
                m_vertex.memory,
                nullptr
            );
        }

        if (m_index.buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(
                device,
                m_index.buffer,
                nullptr
            );
        }

        if (m_index.memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(
                device,
                m_index.memory,
                nullptr
            );
        }

        m_vertex = {};
        m_index = {};

        m_indexCount = 0;
        m_initialized = false;
    }

    // ---------------------------------------------------------
    // Draw
    // ---------------------------------------------------------

    void Draw(VkCommandBuffer commandBuffer) const
    {
        if (!m_initialized)
            return;

        if (m_vertex.buffer == VK_NULL_HANDLE)
            return;

        if (m_index.buffer == VK_NULL_HANDLE)
            return;

        if (m_indexCount == 0)
            return;

        VkBuffer vertexBuffers[] =
        {
            m_vertex.buffer
        };

        VkDeviceSize offsets[] =
        {
            0
        };

        vkCmdBindVertexBuffers(
            commandBuffer,
            0,
            1,
            vertexBuffers,
            offsets
        );

        vkCmdBindIndexBuffer(
            commandBuffer,
            m_index.buffer,
            0,
            VK_INDEX_TYPE_UINT16
        );

        vkCmdDrawIndexed(
            commandBuffer,
            m_indexCount,
            1,
            0,
            0,
            0
        );
    }
};