#pragma once

#include "Core/ApplicationInfo.h"
#include "Core/Liara_App.h"
#include "Core/Liara_GameObject.h"
#include "Listener/KeybordMovementController.h"

#include <memory>

class DemoApp final : public Liara::Core::Liara_App
{
public:
    /**
     * @brief Constructor taking application metadata
     * @param appInfo Application information from LIARA_APPLICATION macro
     */
    explicit DemoApp(const Liara::Core::ApplicationInfo& appInfo);
    ~DemoApp() override = default;

protected:
    void ProcessInput(float frameTime) override;
    void InitSystems() override;

private:
    void LoadGameObjects();

    std::unique_ptr<Liara::Core::Liara_GameObject> m_Player;
    Liara::Listener::KeybordMovementController m_Controller;
};