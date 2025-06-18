#include "DemoApp.h"

#include "Core/ApplicationInfo.h"
#include "Core/ImGui/ImGuiElementEngineStats.h"
#include "Core/Liara_App.h"
#include "Graphics/Liara_Model.h"
#include "Listener/KeybordMovementController.h"
#include "Systems/ImGuiSystem.h"
#include "Systems/PointLightSystem.h"
#include "Systems/SimpleRenderSystem.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "fmt/core.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

DemoApp::DemoApp(const Liara::Core::ApplicationInfo& appInfo)
    : Liara_App(appInfo)
    , m_Controller(*m_SettingsManager) {
    fmt::print("Starting {} v{}\n", appInfo.get_display_name(), appInfo.version.to_string());

    if (!appInfo.description.empty()) { fmt::print("Description: {}\n", appInfo.description); }

    LoadGameObjects();

    m_Player = std::make_unique<Liara::Core::Liara_GameObject>(Liara::Core::Liara_GameObject::CreateGameObject());
    m_Player->transform.position.z = -2.5f;  // NOLINT
}

void DemoApp::ProcessInput(const float frameTime) {
    m_Controller.moveInPlaneXZ(m_Window.GetWindow(), frameTime, *m_Player);
    m_Camera.SetViewYXZ(m_Player->transform.position, m_Player->transform.rotation);
}

void DemoApp::InitSystems() {
    AddSystem(std::make_unique<Liara::Systems::SimpleRenderSystem>(
        m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_GlobalSetLayout, *m_SettingsManager));
    AddSystem(std::make_unique<Liara::Systems::PointLightSystem>(
        m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_GlobalSetLayout, *m_SettingsManager));
    auto imguiSystem = std::make_unique<Liara::Systems::ImGuiSystem>(m_Window,
                                                                     m_Device,
                                                                     m_RendererManager.GetRenderer().GetRenderPass(),
                                                                     m_RendererManager.GetRenderer().GetImageCount());
    imguiSystem->AddElement(std::make_unique<Liara::Core::ImGuiElements::EngineStats>(m_ApplicationInfo));
    AddSystem(std::move(imguiSystem));
}


void DemoApp::LoadGameObjects() {
    const std::shared_ptr model =
        Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/models/viking_room.obj", 1);
    auto vikingRoom = Liara::Core::Liara_GameObject::CreateGameObject();
    vikingRoom.model = model;
    vikingRoom.transform.position = {0.F, .75F, 0.F};
    vikingRoom.transform.scale = {1.5F, 1.5F, 1.5F};
    vikingRoom.transform.rotation = {glm::radians(90.F), glm::radians(135.F), 0.f};
    m_GameObjects.emplace(vikingRoom.GetId(), std::move(vikingRoom));

    const std::vector<glm::vec3> lightColors{
        {1.F, .1F, .1F},
        {.1F, .1F, 1.F},
        {.1F, 1.F, .1F},
        {1.F, 1.F, .1F},
        {.1F, 1.F, 1.F},
    };

    for (size_t i = 0; i < lightColors.size(); i++) {
        auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.5F);
        pointLight.color = lightColors[i];
        auto rotateLight =
            glm::rotate(glm::mat4(1.F),
                        (static_cast<float>(i) * glm::two_pi<float>()) / static_cast<float>(lightColors.size()),
                        {0.F, -1.F, 0.F});
        pointLight.transform.position = glm::vec3(rotateLight * glm::vec4(-1.F, -0.3F, -1.F, 1.F));
        m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
    }

    auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.5F);
    pointLight.color = {1.F, 1.F, 1.F};
    pointLight.transform.position = {0.F, -0.25F, 0.F};
    m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
}