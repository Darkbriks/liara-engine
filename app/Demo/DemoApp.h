#pragma once

#include "Core/Liara_App.h"
#include "Listener/KeybordMovementController.h"

class DemoApp final : public Liara::Core::Liara_App
{
public:
    /**
     * @brief Constructor taking application metadata
     * @param app_info Application information from LIARA_APPLICATION macro
     */
    explicit DemoApp(const Liara::Core::ApplicationInfo& app_info);
    ~DemoApp() override = default;

protected:
    void ProcessInput(float frameTime) override;
    void InitSystems() override;

private:
    void LoadGameObjects();

    std::unique_ptr<Liara::Core::Liara_GameObject> m_Player;
    Liara::Listener::KeybordMovementController m_Controller;
};