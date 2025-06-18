#pragma once

#include "glm/ext/vector_float2.hpp"

namespace Liara::Core::Component
{
    struct RigidBody2dComponent
    {
        glm::vec2 velocity;
        float mass{1.0F};
    };
}