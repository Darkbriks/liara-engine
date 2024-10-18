#pragma once
#include "Liara_Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Liara
{
    class Liara_Model
    {
    public:
        struct Vertex
        {
            glm::vec2 position;
            //glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        };

        Liara_Model(Liara_Device& device, const std::vector<Vertex>& vertices);
        ~Liara_Model();

        Liara_Model(const Liara_Model&) = delete;
        Liara_Model& operator=(const Liara_Model&) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

    private:
        void CreateVertexBuffer(const std::vector<Vertex>& vertices);

        Liara_Device& m_Device;
        VkBuffer m_VertexBuffer;
        VkDeviceMemory m_VertexBufferMemory;
        uint32_t m_VertexCount;
    };
} // Liara
