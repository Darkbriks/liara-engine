//
// Created by antoi on 20/10/2024.
//

#include "SierpinskiApp.h"
#include "Systems/SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

SierpinskiApp::SierpinskiApp()
{
    LoadGameObjects();
}

SierpinskiApp::~SierpinskiApp() = default;

void SierpinskiApp::Run()
{
    Liara::Core::Liara_Camera camera {};
    camera.SetOrthographicProjection(-1, 1, -1, 1, -1, 1);
    const Liara::Systems::SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass()};

    while (!m_Window.ShouldClose())
    {
        glfwPollEvents();

        if (const auto commandBuffer = m_Renderer.BeginFrame())
        {
            const int frameIndex = static_cast<int>(m_Renderer.GetFrameIndex());
            Liara::Core::FrameInfo frameInfo{frameIndex, 0, commandBuffer, camera};

            m_Renderer.BeginSwapChainRenderPass(commandBuffer);
            render_system.RenderGameObjects(frameInfo, m_GameObjects);
            m_Renderer.EndSwapChainRenderPass(commandBuffer);
            m_Renderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void SierpinskiApp::LoadGameObjects()
{
    // A Sierpinski triangle
    Liara::Graphics::Liara_Model::Builder builder{};
    SierpinskiTriangle(builder.vertices, 5, {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    const auto model = std::make_shared<Liara::Graphics::Liara_Model>(m_Device, builder);
    auto triangle = Liara::Core::Liara_GameObject::CreateGameObject();
    triangle.m_Model = model;
    triangle.m_color = {0.0f, 0.8f, 1.0f};
    m_GameObjects.push_back(std::move(triangle));
}

void SierpinskiApp::SierpinskiTriangle(std::vector<Liara::Graphics::Liara_Model::Vertex> &vertices, int depth, Liara::Graphics::Liara_Model::Vertex v0, Liara::Graphics::Liara_Model::Vertex v1, Liara::Graphics::Liara_Model::Vertex v2)
{
    if (depth <= 0)
    {
        vertices.push_back({v0});
        vertices.push_back({v1});
        vertices.push_back({v2});
    }
    else
    {
        // Calculate midpoints of sides, and interpolate color
        Liara::Graphics::Liara_Model::Vertex v01 = {(v0.position + v1.position) / 2.0f, (v0.color + v1.color) / 2.0f};
        Liara::Graphics::Liara_Model::Vertex v12 = {(v1.position + v2.position) / 2.0f, (v1.color + v2.color) / 2.0f};
        Liara::Graphics::Liara_Model::Vertex v20 = {(v2.position + v0.position) / 2.0f, (v2.color + v0.color) / 2.0f};

        SierpinskiTriangle(vertices, depth - 1, v0, v01, v20);
        SierpinskiTriangle(vertices, depth - 1, v01, v1, v12);
        SierpinskiTriangle(vertices, depth - 1, v20, v12, v2);
    }
}