#include "FirstApp.h"
#include "Listener/KeybordMovementController.h"
#include "Core/FrameInfo.h"
#include "Core/ImGui/ImGuiElementEngineStats.h"
#include "Systems/ImGuiSystem.h"
#include "Systems/PointLightSystem.h"
#include "Systems/SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

FirstApp::FirstApp()
{
    LoadGameObjects();

    m_Player = std::make_unique<Liara::Core::Liara_GameObject>(Liara::Core::Liara_GameObject::CreateGameObject());
    m_Player->m_Transform.position.z = -2.5f;
    m_Controller = Liara::Listener::KeybordMovementController{};
}

void FirstApp::ProcessInput(const float frameTime)
{
    m_Controller.moveInPlaneXZ(m_Window.GetWindow(), frameTime, *m_Player);
    m_Camera.SetViewYXZ(m_Player->m_Transform.position, m_Player->m_Transform.rotation);
}

void FirstApp::InitSystems()
{
    AddSystem(std::make_unique<Liara::Systems::SimpleRenderSystem>(m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_GlobalSetLayout));
    AddSystem(std::make_unique<Liara::Systems::PointLightSystem>(m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_GlobalSetLayout));
    auto imguiSystem = std::make_unique<Liara::Systems::ImGuiSystem>(m_Window, m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_RendererManager.GetRenderer().GetImageCount());
    imguiSystem->AddElement(std::make_unique<Liara::Core::ImGuiElements::EngineStats>());
    AddSystem(std::move(imguiSystem));
}


void FirstApp::LoadGameObjects()
{
    std::shared_ptr<Liara::Graphics::Liara_Model> model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/models/viking_room.obj", 1);
    auto viking_room = Liara::Core::Liara_GameObject::CreateGameObject();
    viking_room.m_Model = model;
    viking_room.m_Transform.position = {0.f, .75f, 0.f};
    viking_room.m_Transform.scale = {1.5f, 1.5f, 1.5f};
    viking_room.m_Transform.rotation = {glm::radians(90.f), glm::radians(135.f), 0.f};
    m_GameObjects.emplace(viking_room.GetId(), std::move(viking_room));

    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
    };

    for (int i = 0; i < lightColors.size(); i++)
    {
        auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.5f);
        pointLight.m_color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (static_cast<float>(i) * glm::two_pi<float>()) / static_cast<float>(lightColors.size()),
            {0.f, -1.f, 0.f});
        pointLight.m_Transform.position = glm::vec3(rotateLight * glm::vec4(-1.f, -0.3f, -1.f, 1.f));
        m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
    }

    auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.5f);
    pointLight.m_color = {1.f, 1.f, 1.f};
    pointLight.m_Transform.position = {0.f, -0.25f, 0.f};
    m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
}