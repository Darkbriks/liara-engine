#include "Liara_Model.h"

#include "Core/FrameInfo.h"
#include "Core/Liara_Utils.h"

#include <chrono>
#include <cstring>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#ifndef ENGINE_DIR
    #define ENGINE_DIR "./"
#endif

namespace std
{
    template <> struct hash<Liara::Graphics::Liara_Model::Vertex>
    {
        size_t operator()(const Liara::Graphics::Liara_Model::Vertex& vertex) const noexcept {
            size_t seed = 0;
            Liara::Core::HashCombine(
                seed, vertex.position, vertex.color, vertex.normal, vertex.uv, vertex.specularExponent);
            return seed;
        }
    };
}

namespace Liara::Graphics
{
    Liara_Model::Liara_Model(Liara_Device& device, const Builder& builder)
        : m_Device(device) {
        CreateVertexBuffer(builder.vertices);
        CreateIndexBuffer(builder.indices);
    }

    std::unique_ptr<Liara_Model> Liara_Model::CreateModelFromFile(Liara_Device& device,
                                                                  const std::string& filename,
                                                                  const uint32_t specularExponent) {
        Builder builder{};
        builder.LoadModel(filename, specularExponent);
        return std::make_unique<Liara_Model>(device, builder);
    }

    void Liara_Model::CreateVertexBuffer(const std::vector<Vertex>& vertices) {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3!");
        const VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        constexpr uint32_t vertexSize = sizeof(vertices[0]);

        Liara_Buffer stagingBuffer{m_Device,
                                   vertexSize,
                                   m_VertexCount,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(vertices.data());

        m_VertexBuffer =
            std::make_unique<Liara_Buffer>(m_Device,
                                           vertexSize,
                                           m_VertexCount,
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
    }

    void Liara_Model::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_HasIndexBuffer = m_IndexCount > 0;
        if (!m_HasIndexBuffer) { return; }

        const VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
        constexpr uint32_t indexSize = sizeof(indices[0]);

        Liara_Buffer stagingBuffer{m_Device,
                                   indexSize,
                                   m_IndexCount,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(indices.data());

        m_IndexBuffer =
            std::make_unique<Liara_Buffer>(m_Device,
                                           indexSize,
                                           m_IndexCount,
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
    }

    void Liara_Model::Bind(VkCommandBuffer commandBuffer) const {
        const VkBuffer buffers[] = {m_VertexBuffer->GetBuffer()};
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (m_HasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void Liara_Model::Draw(VkCommandBuffer commandBuffer) const {
        frameStats.triangleCount += m_IndexCount / 3;
        frameStats.vertexCount += m_VertexCount;
        frameStats.drawCallCount++;

        const auto start = std::chrono::high_resolution_clock::now();
        if (m_HasIndexBuffer) { vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0); }
        else { vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0); }
        const auto end = std::chrono::high_resolution_clock::now();

        frameStats.meshDrawTime += std::chrono::duration<float, std::milli>(end - start).count();
    }

    std::vector<VkVertexInputBindingDescription> Liara_Model::Vertex::GetBindingDescriptions() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return {bindingDescription};
    }

    std::vector<VkVertexInputAttributeDescription> Liara_Model::Vertex::GetAttributeDescriptions() {
        return {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)        },
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)           },
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)          },
            {3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv)              },
            {4, 0, VK_FORMAT_R32_UINT,         offsetof(Vertex, specularExponent)}
        };
    }

    void Liara_Model::Builder::LoadModel(const std::string& filename, const uint32_t specularExponent) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, (std::string(ENGINE_DIR) + filename).c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                                       attrib.vertices[3 * index.vertex_index + 1],
                                       attrib.vertices[3 * index.vertex_index + 2]};

                    vertex.color = {attrib.colors[3 * index.vertex_index + 0],
                                    attrib.colors[3 * index.vertex_index + 1],
                                    attrib.colors[3 * index.vertex_index + 2]};

                    vertex.specularExponent = specularExponent;
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                                     attrib.normals[3 * index.normal_index + 1],
                                     attrib.normals[3 * index.normal_index + 2]};
                }

                if (index.texcoord_index >= 0) {
                    // Flip the y coordinate to match with Vulkan's texture coordinate system
                    vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                                 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}