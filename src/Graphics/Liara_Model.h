#pragma once

#include "Liara_Buffer.h"
#include "Liara_Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <memory>

namespace Liara::Graphics
{
    class Liara_Model
    {
    public:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;
            uint32_t specularExponent;  // TODO : Bad way to do this, but it's just for waiting for the texture system

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv
                       && specularExponent == other.specularExponent;
            }
        };

        struct Builder
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            void LoadModel(const std::string& filename, uint32_t specularExponent);
        };

        Liara_Model(Liara_Device& device, const Builder& builder);
        ~Liara_Model() = default;

        Liara_Model(const Liara_Model&) = delete;
        Liara_Model& operator=(const Liara_Model&) = delete;

        // TODO : Remove specularExponent, and use a texture based value
        static std::unique_ptr<Liara_Model>
        CreateModelFromFile(Liara_Device& device, const std::string& filename, uint32_t specularExponent = 1);

        void Bind(VkCommandBuffer commandBuffer) const;
        void Draw(VkCommandBuffer commandBuffer) const;

    private:
        void CreateVertexBuffer(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

        Liara_Device& m_Device;

        std::unique_ptr<Liara_Buffer> m_VertexBuffer;
        uint32_t m_VertexCount{};

        bool m_HasIndexBuffer = false;
        std::unique_ptr<Liara_Buffer> m_IndexBuffer;
        uint32_t m_IndexCount{};
    };
}