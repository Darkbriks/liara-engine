#pragma once
#include "Liara_Camera.h"
#include "Liara_GameObject.h"
#include "Systems/PointLightSystem.h"

#include <vulkan/vulkan_core.h>



namespace Liara::Core
{
    struct FrameInfo
    {
        int frameIndex;
        float deltaTime;
        VkCommandBuffer commandBuffer;
        Liara_Camera& camera;
        VkDescriptorSet globalDescriptorSet;
        Liara_GameObject::Map& gameObjects;
    };

    struct FrameStats
    {
        uint64_t triangleCount = 0;
        uint64_t vertexCount = 0;
        uint64_t drawCallCount = 0;
        double meshDrawTime = 0.0f;

        uint64_t previousTriangleCount = 0;
        uint64_t previousVertexCount = 0;
        uint64_t previousDrawCallCount = 0;
        double previousMeshDrawTime = 0.0f;

        void Reset() {
            previousTriangleCount = triangleCount;
            previousVertexCount = vertexCount;
            previousDrawCallCount = drawCallCount;
            previousMeshDrawTime = meshDrawTime;

            triangleCount = 0;
            vertexCount = 0;
            drawCallCount = 0;
            meshDrawTime = 0.0f;
        }
    };
}

namespace Liara
{
    // TODO: Use a more robust frame stats system
    inline Core::FrameStats frameStats{};
}