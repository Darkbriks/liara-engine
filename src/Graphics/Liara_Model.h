#pragma once

#include <vulkan/vulkan_core.h>

#include <concepts>
#include <memory>
#include <span>
#include <string_view>

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "Liara_Buffer.h"
#include "Liara_Device.h"

namespace Liara::Graphics
{

    template <typename T>
    concept VertexType = requires {
        typename T::position_type;
        typename T::color_type;
        typename T::normal_type;
        typename T::uv_type;
        requires std::is_trivially_copyable_v<T>;
        { T::GetBindingDescriptions() } -> std::convertible_to<std::vector<VkVertexInputBindingDescription>>;
        { T::GetAttributeDescriptions() } -> std::convertible_to<std::vector<VkVertexInputAttributeDescription>>;
    };

    class Liara_Model
    {
    public:
        struct Vertex
        {
            using position_type = glm::vec3;
            using color_type = glm::vec3;
            using normal_type = glm::vec3;
            using uv_type = glm::vec2;

            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};
            uint32_t specularExponent{1};  // TODO: Remove when material system ready

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv
                       && specularExponent == other.specularExponent;
            }
        };

        static_assert(VertexType<Vertex>, "Vertex must satisfy VertexType concept");

        /**
         * @brief Create model from raw vertex/index data
         * @param device Vulkan device
         * @param vertices Vertex data span
         * @param indices Index data span (optional)
         * @return Unique pointer to model
         */
        static std::unique_ptr<Liara_Model>
        CreateFromData(Liara_Device& device, std::span<const Vertex> vertices, std::span<const uint32_t> indices = {});

        /**
         * @brief Create model from file (OBJ format)
         * @param device Vulkan device
         * @param filename Path to model file
         * @param specularExponent Default specular value (TODO: remove)
         * @return Unique pointer to model
         */
        static std::unique_ptr<Liara_Model>
        CreateFromFile(Liara_Device& device, std::string_view filename, uint32_t specularExponent = 1);

        /**
         * @brief Create basic geometric primitives
         */
        struct Primitives
        {
            static std::unique_ptr<Liara_Model> CreateQuad(Liara_Device& device);
            static std::unique_ptr<Liara_Model> CreateCube(Liara_Device& device);
            static std::unique_ptr<Liara_Model> CreateSphere(Liara_Device& device, uint32_t segments = 32);
            static std::unique_ptr<Liara_Model> CreatePlane(Liara_Device& device, float size = 1.0f);
            static std::unique_ptr<Liara_Model>
            CreateCylinder(Liara_Device& device, float height = 1.0f, uint32_t segments = 32);
        };

        ~Liara_Model() = default;
        Liara_Model(const Liara_Model&) = delete;
        Liara_Model& operator=(const Liara_Model&) = delete;

        void Bind(VkCommandBuffer commandBuffer) const;
        void Draw(VkCommandBuffer commandBuffer) const;

        [[nodiscard]] uint32_t GetVertexCount() const noexcept { return m_VertexCount; }
        [[nodiscard]] uint32_t GetIndexCount() const noexcept { return m_IndexCount; }
        [[nodiscard]] bool HasIndices() const noexcept { return m_HasIndexBuffer; }
        [[nodiscard]] size_t GetTriangleCount() const noexcept {
            return HasIndices() ? m_IndexCount / 3 : m_VertexCount / 3;
        }

    private:
        explicit Liara_Model(Liara_Device& device, std::span<const Vertex> vertices, std::span<const uint32_t> indices);

        void CreateVertexBuffer(std::span<const Vertex> vertices);
        void CreateIndexBuffer(std::span<const uint32_t> indices);

        Liara_Device& m_Device;

        std::unique_ptr<Liara_Buffer> m_VertexBuffer;
        uint32_t m_VertexCount{};

        std::unique_ptr<Liara_Buffer> m_IndexBuffer;
        uint32_t m_IndexCount{};
        bool m_HasIndexBuffer{false};
    };

    /**
     * @brief Intermediate representation for mesh data loading
     * Used internally by file loaders, not exposed to users
     */
    struct MeshData
    {
        std::vector<Liara_Model::Vertex> vertices;
        std::vector<uint32_t> indices;

        [[nodiscard]] std::span<const Liara_Model::Vertex> GetVertices() const noexcept { return vertices; }
        [[nodiscard]] std::span<const uint32_t> GetIndices() const noexcept { return indices; }

        [[nodiscard]] bool Empty() const noexcept { return vertices.empty(); }

        void Clear() noexcept {
            vertices.clear();
            indices.clear();
        }
    };

    /**
     * @brief Load mesh data from OBJ file
     * @param filename Path to OBJ file
     * @param specularExponent Default specular value
     * @return MeshData structure
     */
    [[nodiscard]] MeshData LoadMeshFromOBJ(std::string_view filename, uint32_t specularExponent = 1);
}