#pragma once

#include <glm/gtc/matrix_transform.hpp>

namespace Liara::Core::Component
{
    struct RigidBody2dComponent
    {
        glm::vec2 velocity;
        float mass{1.0f};
    };
}