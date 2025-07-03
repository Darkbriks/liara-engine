#include "PrimitiveGenerator.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Liara::Graphics::PrimitiveGenerator
{
    MeshData GenerateQuad() {
        return MeshData{
            .vertices = {{.position = {-0.5f, -0.5f, 0.0f},
                          .color = {1.0f, 0.0f, 0.0f},
                          .normal = {0.0f, 0.0f, 1.0f},
                          .uv = {0.0f, 0.0f},
                          .specularExponent = 1},
                         {.position = {0.5f, -0.5f, 0.0f},
                          .color = {0.0f, 1.0f, 0.0f},
                          .normal = {0.0f, 0.0f, 1.0f},
                          .uv = {1.0f, 0.0f},
                          .specularExponent = 1},
                         {.position = {0.5f, 0.5f, 0.0f},
                          .color = {0.0f, 0.0f, 1.0f},
                          .normal = {0.0f, 0.0f, 1.0f},
                          .uv = {1.0f, 1.0f},
                          .specularExponent = 1},
                         {.position = {-0.5f, 0.5f, 0.0f},
                          .color = {1.0f, 1.0f, 0.0f},
                          .normal = {0.0f, 0.0f, 1.0f},
                          .uv = {0.0f, 1.0f},
                          .specularExponent = 1}},
            .indices = {0, 1, 2, 2, 3, 0}
        };
    }

    MeshData GenerateCube() {
        MeshData meshData;
        meshData.vertices.reserve(24);
        meshData.indices.reserve(36);

        struct Face
        {
            glm::vec3 positions[4];
            glm::vec3 normal;
            glm::vec2 uvs[4];
        };

        for (int faceIdx = 0; faceIdx < 6; ++faceIdx) {
            const Face faces[6] = {
                // Front face (Z+)
                {.positions = {{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
                 .normal = {0.0f, 0.0f, 1.0f},
                 .uvs = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}},

                // Back face (Z-)
                {.positions = {{0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}},
                 .normal = {0.0f, 0.0f, -1.0f},
                 .uvs = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}},

                // Left face (X-)
                {.positions = {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}},
                 .normal = {-1.0f, 0.0f, 0.0f},
                 .uvs = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}},

                // Right face (X+)
                {.positions = {{0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
                 .normal = {1.0f, 0.0f, 0.0f},
                 .uvs = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}},

                // Top face (Y+)
                {.positions = {{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}},
                 .normal = {0.0f, 1.0f, 0.0f},
                 .uvs = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}},

                // Bottom face (Y-)
                {.positions = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}},
                 .normal = {0.0f, -1.0f, 0.0f},
                 .uvs = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}}
            };
            const auto& [positions, normal, uvs] = faces[faceIdx];
            const auto baseVertexIndex = static_cast<uint32_t>(meshData.vertices.size());

            for (int vertIdx = 0; vertIdx < 4; ++vertIdx) {
                constexpr glm::vec3 colors[6] = {
                    {1.0f, 0.0f, 0.0f}, // Front - Rouge
                    {0.0f, 1.0f, 0.0f}, // Back - Vert
                    {0.0f, 0.0f, 1.0f}, // Left - Bleu
                    {1.0f, 1.0f, 0.0f}, // Right - Jaune
                    {1.0f, 0.0f, 1.0f}, // Top - Magenta
                    {0.0f, 1.0f, 1.0f}  // Bottom - Cyan
                };
                meshData.vertices.push_back({.position = positions[vertIdx],
                                             .color = colors[faceIdx],
                                             .normal = normal,
                                             .uv = uvs[vertIdx],
                                             .specularExponent = 1});
            }

            for (const uint32_t indices[6] = {0, 1, 2, 2, 3, 0}; const uint32_t idx : indices) {
                meshData.indices.push_back(baseVertexIndex + idx);
            }
        }

        return meshData;
    }

    MeshData GenerateSphere(uint32_t segments) {
        segments = std::max(segments, 8u);
        segments = std::min(segments, 256u);

        const uint32_t rings = segments / 2;
        const uint32_t sectors = segments;

        MeshData meshData;

        const size_t vertexCount = (rings + 1) * (sectors + 1);
        const size_t indexCount = rings * sectors * 6;

        meshData.vertices.reserve(vertexCount);
        meshData.indices.reserve(indexCount);

        for (uint32_t ring = 0; ring <= rings; ++ring) {
            const float phi = glm::pi<float>() * static_cast<float>(ring) / static_cast<float>(rings);
            const float y = std::cos(phi);
            const float sinPhi = std::sin(phi);

            for (uint32_t sector = 0; sector <= sectors; ++sector) {
                const float theta = 2.0f * glm::pi<float>() * static_cast<float>(sector) / static_cast<float>(sectors);
                const float x = sinPhi * std::cos(theta);
                const float z = sinPhi * std::sin(theta);

                const glm::vec3 position{x, y, z};
                const glm::vec3 normal = glm::normalize(position);

                const float u = static_cast<float>(sector) / static_cast<float>(sectors);
                const float v = static_cast<float>(ring) / static_cast<float>(rings);

                const glm::vec3 color{0.5f + 0.5f * normal.x, 0.5f + 0.5f * normal.y, 0.5f + 0.5f * normal.z};

                meshData.vertices.push_back({
                    .position = position, .color = color, .normal = normal, .uv = {u, v},
                               .specularExponent = 32
                });
            }
        }

        for (uint32_t ring = 0; ring < rings; ++ring) {
            for (uint32_t sector = 0; sector < sectors; ++sector) {
                const uint32_t current = (ring * (sectors + 1)) + sector;
                const uint32_t next = current + sectors + 1;

                meshData.indices.push_back(current);
                meshData.indices.push_back(next);
                meshData.indices.push_back(current + 1);

                meshData.indices.push_back(current + 1);
                meshData.indices.push_back(next);
                meshData.indices.push_back(next + 1);
            }
        }

        return meshData;
    }

    MeshData GeneratePlane(const float size) {
        const float halfSize = size * 0.5f;

        return MeshData{
            .vertices = {{.position = {-halfSize, 0.0f, -halfSize},
                          .color = {0.8f, 0.8f, 0.8f},
                          .normal = {0.0f, 1.0f, 0.0f},
                          .uv = {0.0f, 0.0f},
                          .specularExponent = 1},
                         {.position = {halfSize, 0.0f, -halfSize},
                          .color = {0.8f, 0.8f, 0.8f},
                          .normal = {0.0f, 1.0f, 0.0f},
                          .uv = {1.0f, 0.0f},
                          .specularExponent = 1},
                         {.position = {halfSize, 0.0f, halfSize},
                          .color = {0.8f, 0.8f, 0.8f},
                          .normal = {0.0f, 1.0f, 0.0f},
                          .uv = {1.0f, 1.0f},
                          .specularExponent = 1},
                         {.position = {-halfSize, 0.0f, halfSize},
                          .color = {0.8f, 0.8f, 0.8f},
                          .normal = {0.0f, 1.0f, 0.0f},
                          .uv = {0.0f, 1.0f},
                          .specularExponent = 1}},
            .indices = {0, 1, 2, 2, 3, 0}
        };
    }
}