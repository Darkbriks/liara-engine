#pragma once

#include "Liara_Device.h"
#include "Liara_Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

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

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

            bool operator==(const Vertex& other) const
            {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void LoadModel(const std::string& filename);
        };

        Liara_Model(Liara_Device& device, const Builder& builder);
        ~Liara_Model() = default;

        Liara_Model(const Liara_Model&) = delete;
        Liara_Model& operator=(const Liara_Model&) = delete;

        static std::unique_ptr<Liara_Model> CreateModelFromFile(Liara_Device& device, const std::string& filename);

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
} // Liara
