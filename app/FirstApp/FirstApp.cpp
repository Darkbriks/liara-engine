#include "FirstApp.h"
#include "Listener/KeybordMovementController.h"
#include "Core/FrameInfo.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

FirstApp::FirstApp() : Liara_App("First App", 800, 600)
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

void FirstApp::LoadGameObjects()
{
    std::shared_ptr<Liara::Graphics::Liara_Model> model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/models/smooth_vase.obj", 32);
    auto flatVase = Liara::Core::Liara_GameObject::CreateGameObject();
    flatVase.m_Model = model;
    flatVase.m_Transform.position = {-.5f, .5f, 0.f};
    flatVase.m_Transform.scale = {3.f, 1.5f, 3.f};
    m_GameObjects.emplace(flatVase.GetId(), std::move(flatVase));

    model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/models/smooth_vase.obj", 2048);
    auto smoothVase = Liara::Core::Liara_GameObject::CreateGameObject();
    smoothVase.m_Model = model;
    smoothVase.m_Transform.position = {.5f, .5f, 0.f};
    smoothVase.m_Transform.scale = {3.f, 1.5f, 3.f};
    m_GameObjects.emplace(smoothVase.GetId(), std::move(smoothVase));

    model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/models/quad.obj", 512);
    auto floor = Liara::Core::Liara_GameObject::CreateGameObject();
    floor.m_Model = model;
    floor.m_Transform.position = {0.f, .5f, 0.f};
    floor.m_Transform.scale = {3.f, 1.f, 3.f};
    m_GameObjects.emplace(floor.GetId(), std::move(floor));

    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        //{1.f, 1.f, 1.f}
    };

    for (int i = 0; i < lightColors.size(); i++)
    {
        auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.2f);
        pointLight.m_color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (static_cast<float>(i) * glm::two_pi<float>()) / static_cast<float>(lightColors.size()),
            {0.f, -1.f, 0.f});
        pointLight.m_Transform.position = glm::vec3(rotateLight * glm::vec4(-1.f, -0.3f, -1.f, 1.f));
        m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
    }
}