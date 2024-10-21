#include "Liara_Model.h"

namespace Liara
{
    Liara_Model::Liara_Model(Liara_Device& device, const std::vector<Vertex>& vertices) : m_Device(device)
    {
        CreateVertexBuffer(vertices);
    }

    Liara_Model::~Liara_Model()
    {
        vkDestroyBuffer(m_Device.GetDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(m_Device.GetDevice(), m_VertexBufferMemory, nullptr);
    }

    void Liara_Model::CreateVertexBuffer(const std::vector<Vertex> &vertices)
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3!");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VertexBuffer, m_VertexBufferMemory);

        void* data;
        vkMapMemory(m_Device.GetDevice(), m_VertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device.GetDevice(), m_VertexBufferMemory);
    }

    void Liara_Model::Bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = { m_VertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void Liara_Model::Draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }

    std::vector<VkVertexInputBindingDescription> Liara_Model::Vertex::GetBindingDescriptions()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return { bindingDescription };
    }

    std::vector<VkVertexInputAttributeDescription> Liara_Model::Vertex::GetAttributeDescriptions()
    {
        return{
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}
        };
    }
} // Liara