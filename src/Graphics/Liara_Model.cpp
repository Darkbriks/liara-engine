#include "Liara_Model.h"

#include "Core/FrameInfo.h"
#include "Graphics/Liara_Buffer.h"
#include "Graphics/Liara_Device.h"

#ifdef LIARA_UTILS_MODULE_AVAILABLE
import liara.core.utils;
#else
    #include "Core/Liara_Utils.h"
#endif

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <chrono>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "PrimitiveGenerator.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#ifndef ENGINE_DIR
    #define ENGINE_DIR "./"
#endif

template <> struct std::hash<Liara::Graphics::Liara_Model::Vertex>
{
    size_t operator()(const Liara::Graphics::Liara_Model::Vertex& vertex) const noexcept {
        size_t seed = 0;
        Liara::Core::HashCombine(
            seed, vertex.position, vertex.color, vertex.normal, vertex.uv, vertex.specularExponent);
        return seed;
    }
};

namespace Liara::Graphics
{
    std::unique_ptr<Liara_Model> Liara_Model::CreateFromData(Liara_Device& device,
                                                             const std::span<const Vertex> vertices,
                                                             const std::span<const uint32_t> indices) {
        LIARA_CHECK_ARGUMENT(!vertices.empty(), LogCore, "Vertices cannot be empty");
        LIARA_CHECK_ARGUMENT(vertices.size() >= 3, LogCore, "At least 3 vertices required");
        return std::unique_ptr<Liara_Model>(new Liara_Model(device, vertices, indices));
    }

    std::unique_ptr<Liara_Model> Liara_Model::CreateFromFile(Liara_Device& device,
                                                             const std::string_view filename,
                                                             const uint32_t specularExponent) {
        const auto meshData = LoadMeshFromOBJ(filename, specularExponent);
        LIARA_CHECK_RUNTIME(!meshData.Empty(), LogCore, "Failed to load model from file: {}", std::string(filename));

        return CreateFromData(device, meshData.GetVertices(), meshData.GetIndices());
    }

    // === PRIMITIVES ===

    std::unique_ptr<Liara_Model> Liara_Model::Primitives::CreateQuad(Liara_Device& device) {
        const auto meshData = PrimitiveGenerator::GenerateQuad();
        return CreateFromData(device, meshData.GetVertices(), meshData.GetIndices());
    }

    std::unique_ptr<Liara_Model> Liara_Model::Primitives::CreateCube(Liara_Device& device) {
        const auto meshData = PrimitiveGenerator::GenerateCube();
        return CreateFromData(device, meshData.GetVertices(), meshData.GetIndices());
    }

    std::unique_ptr<Liara_Model> Liara_Model::Primitives::CreateSphere(Liara_Device& device, const uint32_t segments) {
        const auto meshData = PrimitiveGenerator::GenerateSphere(segments);
        return CreateFromData(device, meshData.GetVertices(), meshData.GetIndices());
    }

    std::unique_ptr<Liara_Model> Liara_Model::Primitives::CreatePlane(Liara_Device& device, const float size) {
        const auto meshData = PrimitiveGenerator::GeneratePlane(size);
        return CreateFromData(device, meshData.GetVertices(), meshData.GetIndices());
    }

    std::unique_ptr<Liara_Model>
    Liara_Model::Primitives::CreateCylinder(Liara_Device& device, const float height, const uint32_t segments) {
        const auto meshData = PrimitiveGenerator::GenerateCylinder(height, segments);
        return CreateFromData(device, meshData.GetVertices(), meshData.GetIndices());
    }

    // === CONSTRUCTOR ===

    Liara_Model::Liara_Model(Liara_Device& device,
                             const std::span<const Vertex> vertices,
                             const std::span<const uint32_t> indices)
        : m_Device(device) {
        CreateVertexBuffer(vertices);
        CreateIndexBuffer(indices);
    }

    // === CORE METHODS ===

    void Liara_Model::CreateVertexBuffer(std::span<const Vertex> vertices) {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3!");

        m_VertexBuffer = std::make_unique<Liara_Buffer>(m_Device, vertices, BufferConfig::Vertex());
    }

    void Liara_Model::CreateIndexBuffer(std::span<const uint32_t> indices) {
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_HasIndexBuffer = m_IndexCount > 0;
        if (!m_HasIndexBuffer) { return; }

        m_IndexBuffer = std::make_unique<Liara_Buffer>(m_Device, indices, BufferConfig::Index());
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
        frameStats.triangleCount += GetTriangleCount();
        frameStats.vertexCount += m_VertexCount;
        frameStats.drawCallCount++;

        const auto start = std::chrono::high_resolution_clock::now();

        if (m_HasIndexBuffer) { vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0); }
        else { vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0); }

        const auto end = std::chrono::high_resolution_clock::now();
        frameStats.meshDrawTime += std::chrono::duration<float, std::milli>(end - start).count();
    }

    // === VERTEX DESCRIPTIONS ===

    std::vector<VkVertexInputBindingDescription> Liara_Model::Vertex::GetBindingDescriptions() {
        return {
            {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}
        };
    }

    std::vector<VkVertexInputAttributeDescription> Liara_Model::Vertex::GetAttributeDescriptions() {
        return {
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)        },
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)           },
            {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal)          },
            {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT,    .offset = offsetof(Vertex, uv)              },
            {.location = 4, .binding = 0, .format = VK_FORMAT_R32_UINT,         .offset = offsetof(Vertex, specularExponent)}
        };
    }

    // === FILE LOADING ===

    MeshData LoadMeshFromOBJ(std::string_view filename, uint32_t specularExponent) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

        const std::string fullPath = std::string(ENGINE_DIR) + std::string(filename);

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fullPath.c_str())) {
            LIARA_LOG_ERROR(LogCore, "Failed to load OBJ file '{}': {}", fullPath, warn + err);
            return {};
        }

        MeshData meshData;
        std::unordered_map<Liara_Model::Vertex, uint32_t> uniqueVertices;

        for (const auto& shape : shapes) {
            for (const auto& [vertex_index, normal_index, texcoord_index] : shape.mesh.indices) {
                Liara_Model::Vertex vertex{};

                if (vertex_index >= 0) {
                    const auto vid = static_cast<size_t>(vertex_index);
                    vertex.position = {
                        attrib.vertices[(3 * vid) + 0], attrib.vertices[(3 * vid) + 1], attrib.vertices[(3 * vid) + 2]};

                    if (vid < attrib.colors.size() / 3) {
                        vertex.color = {
                            attrib.colors[(3 * vid) + 0], attrib.colors[(3 * vid) + 1], attrib.colors[(3 * vid) + 2]};
                    }
                    else {
                        vertex.color = {1.0f, 1.0f, 1.0f};  // Default white
                    }
                }

                if (normal_index >= 0) {
                    const auto nid = static_cast<size_t>(normal_index);
                    vertex.normal = {
                        attrib.normals[(3 * nid) + 0], attrib.normals[(3 * nid) + 1], attrib.normals[(3 * nid) + 2]};
                }

                if (texcoord_index >= 0) {
                    const auto tid = static_cast<size_t>(texcoord_index);
                    vertex.uv = {
                        attrib.texcoords[(2 * tid) + 0],
                        1.0f - attrib.texcoords[(2 * tid) + 1]  // Flip Y for Vulkan
                    };
                }

                vertex.specularExponent = specularExponent;

                // Deduplicate vertices
                if (auto it = uniqueVertices.find(vertex); it != uniqueVertices.end()) {
                    meshData.indices.push_back(it->second);
                }
                else {
                    const auto newIndex = static_cast<uint32_t>(meshData.vertices.size());
                    uniqueVertices[vertex] = newIndex;
                    meshData.vertices.push_back(vertex);
                    meshData.indices.push_back(newIndex);
                }
            }
        }

        return meshData;
    }
}