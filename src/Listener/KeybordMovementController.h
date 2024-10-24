//
// Created by antoi on 23/10/2024.
//

#pragma once
#include <GLFW/glfw3.h>
#include "Core/Liara_GameObject.h"

namespace Liara::Listener
{
    class KeybordMovementController
    {
    public:
        struct KeyMappings
        {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        // TODO : Utiliser une couche d'abstraction pour les inputs, et non pas directement GLFW (pourrait être utile pour les autres types de périphériques, comme les manettes)
        void moveInPlaneXZ(GLFWwindow* window, float deltaTime, Core::Liara_GameObject& gameObject);

        KeyMappings m_KeyMappings{};
        float m_MoveSpeed = 3.0f;
        float m_LookSpeed = 1.5f;
    };
} // Liara
