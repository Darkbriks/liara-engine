#include "KeybordMovementController.h"
#include "Core/Liara_GameObject.h"

#include <glm/vec3.hpp>

namespace Liara::Listener
{
    void KeybordMovementController::moveInPlaneXZ(SDL_Window */*window*/, float deltaTime, Core::Liara_GameObject &gameObject) const
    {
        const Uint8 *state = SDL_GetKeyboardState(nullptr);

        glm::vec3 rotation{0};
        if (state[m_KeyMappings.lookRight]) { rotation.y += 1.0f; }
        if (state[m_KeyMappings.lookLeft]) { rotation.y -= 1.0f; }
        if (state[m_KeyMappings.lookUp]) { rotation.x += 1.0f; }
        if (state[m_KeyMappings.lookDown]) { rotation.x -= 1.0f; }

        if (dot(rotation, rotation) > std::numeric_limits<float>::epsilon())
        {
            gameObject.m_Transform.rotation += normalize(rotation) * m_LookSpeed * deltaTime;
            gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -1.5f, 1.5f);
            gameObject.m_Transform.rotation.y = glm::mod(gameObject.m_Transform.rotation.y, glm::two_pi<float>());
        }

        const float yaw = gameObject.m_Transform.rotation.y;
        const glm::vec3 forwardDirection{glm::sin(yaw), 0.0f, glm::cos(yaw)};
        const glm::vec3 rightDirection{forwardDirection.z, 0.0f, -forwardDirection.x};
        constexpr glm::vec3 upDirection{0.0f, -1.0f, 0.0f};

        glm::vec3 movement{0};
        if (state[m_KeyMappings.moveForward]) { movement += forwardDirection; }
        if (state[m_KeyMappings.moveBackward]) { movement -= forwardDirection; }
        if (state[m_KeyMappings.moveRight]) { movement += rightDirection; }
        if (state[m_KeyMappings.moveLeft]) { movement -= rightDirection; }
        if (state[m_KeyMappings.moveUp]) { movement += upDirection; }
        if (state[m_KeyMappings.moveDown]) { movement -= upDirection; }

        if (dot(movement, movement) > std::numeric_limits<float>::epsilon())
        {
            gameObject.m_Transform.position += normalize(movement) * m_MoveSpeed * deltaTime;
        }

        // Si F10 est appuyé, on change le mode de VSync
        if (state[SDL_SCANCODE_F10])
        {
            if (!m_F10Pressed)
            {
                m_F10Pressed = true;
                m_SettingsManager.SetBool("graphics.vsync", !m_SettingsManager.GetBool("graphics.vsync"));
            }
        }
        else { m_F10Pressed = false; }

        // Si F11 est appuyé, on change le mode plein écran
        if (state[SDL_SCANCODE_F11])
        {
            if (!m_F11Pressed)
            {
                m_F11Pressed = true;
                // TODO: Change in focused window
                auto windowSettings = m_SettingsManager.Get<Plateform::WindowSettings>("window.0");
                windowSettings.SetFullscreen(!windowSettings.IsFullscreen());
                m_SettingsManager.Set("window.0", windowSettings);
            }
        }
        else { m_F11Pressed = false; }
    }
}