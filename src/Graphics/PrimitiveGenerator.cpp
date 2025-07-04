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

    MeshData GenerateCylinder(const float height, uint32_t segments) {
        segments = std::clamp(segments, 3u, 128u);
        const float halfHeight = height * 0.5f;

        MeshData meshData;

        const size_t vertexCount = (segments * 4) + 2;  // sides(2*segments) + caps(2*segments) + centers(2)
        const size_t indexCount = segments * 12;        // sides(6*segments) + caps(6*segments)

        meshData.vertices.reserve(vertexCount);
        meshData.indices.reserve(indexCount);

        for (uint32_t i = 0; i <= segments; ++i) {
            const float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
            const float x = std::cos(angle);
            const float z = std::sin(angle);
            const float u = static_cast<float>(i) / static_cast<float>(segments);

            meshData.vertices.push_back({
                .position = {x, -halfHeight, z},
                .color = {0.7f, 0.5f, 0.3f},
                .normal = {x, 0.0f, z},
                .uv = {u, 0.0f},
                .specularExponent = 16
            });

            meshData.vertices.push_back({
                .position = {x, halfHeight, z},
                .color = {0.7f, 0.5f, 0.3f},
                .normal = {x, 0.0f, z},
                .uv = {u, 1.0f},
                .specularExponent = 16
            });
        }

        for (uint32_t i = 0; i < segments; ++i) {
            const uint32_t bottom1 = i * 2;
            const uint32_t top1 = bottom1 + 1;
            const uint32_t bottom2 = (i + 1) * 2;
            const uint32_t top2 = bottom2 + 1;

            meshData.indices.insert(meshData.indices.end(), {bottom1, bottom2, top1});
            meshData.indices.insert(meshData.indices.end(), {top1, bottom2, top2});
        }

        const auto sideVertexCount = static_cast<uint32_t>(meshData.vertices.size());

        const uint32_t bottomCenterIdx = sideVertexCount;
        meshData.vertices.push_back({
            .position = {0.0f, -halfHeight, 0.0f},
            .color = {0.8f, 0.6f, 0.4f},
            .normal = {0.0f, -1.0f, 0.0f},
            .uv = {0.5f, 0.5f},
            .specularExponent = 16
        });

        const uint32_t topCenterIdx = bottomCenterIdx + 1;
        meshData.vertices.push_back({
            .position = {0.0f, halfHeight, 0.0f},
            .color = {0.8f, 0.6f, 0.4f},
            .normal = {0.0f, 1.0f, 0.0f},
            .uv = {0.5f, 0.5f},
            .specularExponent = 16
        });

        for (uint32_t i = 0; i < segments; ++i) {
            const float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
            const float x = std::cos(angle);
            const float z = std::sin(angle);
            const float u = 0.5f + 0.5f * x;
            const float v = 0.5f + 0.5f * z;

            meshData.vertices.push_back({
                .position = {x, -halfHeight, z},
                .color = {0.8f, 0.6f, 0.4f},
                .normal = {0.0f, -1.0f, 0.0f},
                .uv = {u, v},
                .specularExponent = 16
            });

            meshData.vertices.push_back({
                .position = {x, halfHeight, z},
                .color = {0.8f, 0.6f, 0.4f},
                .normal = {0.0f, 1.0f, 0.0f},
                .uv = {u, v},
                .specularExponent = 16
            });
        }

        for (uint32_t i = 0; i < segments; ++i) {
            const uint32_t bottomIdx1 = topCenterIdx + 1 + (i * 2);
            const uint32_t bottomIdx2 = topCenterIdx + 1 + (((i + 1) % segments) * 2);
            const uint32_t topIdx1 = bottomIdx1 + 1;
            const uint32_t topIdx2 = bottomIdx2 + 1;

            meshData.indices.insert(meshData.indices.end(), {bottomCenterIdx, bottomIdx2, bottomIdx1});
            meshData.indices.insert(meshData.indices.end(), {topCenterIdx, topIdx1, topIdx2});
        }

        return meshData;
    }
}