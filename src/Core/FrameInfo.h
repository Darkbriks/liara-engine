#pragma once

#include "Liara_Camera.h"
#include "Liara_GameObject.h"

namespace Liara::Core
{
    struct FrameInfo
    {
        int m_FrameIndex;
        float m_DeltaTime;
        VkCommandBuffer m_CommandBuffer;
        Liara_Camera &m_Camera;
        VkDescriptorSet m_GlobalDescriptorSet;
        Liara_GameObject::Map &m_GameObjects;
    };

    struct FrameStats
    {
        uint64_t m_TriangleCount = 0;
        uint64_t m_VertexCount = 0;
        uint64_t m_DrawCallCount = 0;
        double m_MeshDrawTime = 0.0f;

        uint64_t m_PreviousTriangleCount = 0;
        uint64_t m_PreviousVertexCount = 0;
        uint64_t m_PreviousDrawCallCount = 0;
        double m_PreviousMeshDrawTime = 0.0f;

        void Reset()
        {
            m_PreviousTriangleCount = m_TriangleCount;
            m_PreviousVertexCount = m_VertexCount;
            m_PreviousDrawCallCount = m_DrawCallCount;
            m_PreviousMeshDrawTime = m_MeshDrawTime;

            m_TriangleCount = 0;
            m_VertexCount = 0;
            m_DrawCallCount = 0;
            m_MeshDrawTime = 0.0f;
        }
    };
}

namespace Liara
{
    inline Core::FrameStats g_FrameStats{};
}