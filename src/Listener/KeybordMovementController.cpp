//
// Created by antoi on 23/10/2024.
//

#include "KeybordMovementController.h"
#include "Core/Liara_GameObject.h"

#include <glm/vec3.hpp>

namespace Liara::Listener
{
    void KeybordMovementController::moveInPlaneXZ(GLFWwindow *window, float deltaTime, Core::Liara_GameObject &gameObject)
    {
        glm::vec3 rotation{0};
        if (glfwGetKey(window, m_KeyMappings.lookRight) == GLFW_PRESS) { rotation.y += 1.0f; }
        if (glfwGetKey(window, m_KeyMappings.lookLeft) == GLFW_PRESS) { rotation.y -= 1.0f; }
        if (glfwGetKey(window, m_KeyMappings.lookUp) == GLFW_PRESS) { rotation.x += 1.0f; }
        if (glfwGetKey(window, m_KeyMappings.lookDown) == GLFW_PRESS) { rotation.x -= 1.0f; }

        if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon())
        {
            gameObject.m_Transform.rotation += glm::normalize(rotation) * m_LookSpeed * deltaTime;
            gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -1.5f, 1.5f);
            gameObject.m_Transform.rotation.y = glm::mod(gameObject.m_Transform.rotation.y, glm::two_pi<float>());
        }

        const float yaw = gameObject.m_Transform.rotation.y;
        const glm::vec3 forwardDirection{glm::sin(yaw), 0.0f, glm::cos(yaw)};
        const glm::vec3 rightDirection{forwardDirection.z, 0.0f, -forwardDirection.x};
        const glm::vec3 upDirection{0.0f, -1.0f, 0.0f};

        glm::vec3 movement{0};
        if (glfwGetKey(window, m_KeyMappings.moveForward) == GLFW_PRESS) { movement += forwardDirection; }
        if (glfwGetKey(window, m_KeyMappings.moveBackward) == GLFW_PRESS) { movement -= forwardDirection; }
        if (glfwGetKey(window, m_KeyMappings.moveRight) == GLFW_PRESS) { movement += rightDirection; }
        if (glfwGetKey(window, m_KeyMappings.moveLeft) == GLFW_PRESS) { movement -= rightDirection; }
        if (glfwGetKey(window, m_KeyMappings.moveUp) == GLFW_PRESS) { movement += upDirection; }
        if (glfwGetKey(window, m_KeyMappings.moveDown) == GLFW_PRESS) { movement -= upDirection; }

        if (glm::dot(movement, movement) > std::numeric_limits<float>::epsilon())
        {
            gameObject.m_Transform.position += glm::normalize(movement) * m_MoveSpeed * deltaTime;
        }
    }

} // Liara