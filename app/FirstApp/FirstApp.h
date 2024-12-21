#pragma once

#include "Core/Liara_App.h"

#include "Listener/KeybordMovementController.h"

class FirstApp final : public Liara::Core::Liara_App
{
public:
    FirstApp();
    ~FirstApp() override = default;

protected:
    void ProcessInput(float frameTime) override;

private:
    void LoadGameObjects();

    std::unique_ptr<Liara::Core::Liara_GameObject> m_Player;
    Liara::Listener::KeybordMovementController m_Controller;
};