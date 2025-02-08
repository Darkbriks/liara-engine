#pragma once
#include <SDL2/SDL.h>
#include "Core/Liara_GameObject.h"

namespace Liara::Listener
{
    class KeybordMovementController
    {
    public:
        struct KeyMappings
        {
            int moveLeft = SDL_SCANCODE_A;
            int moveRight = SDL_SCANCODE_D;
            int moveForward = SDL_SCANCODE_W;
            int moveBackward = SDL_SCANCODE_S;
            int moveUp = SDL_SCANCODE_E;
            int moveDown = SDL_SCANCODE_Q;
            int lookLeft = SDL_SCANCODE_LEFT;
            int lookRight = SDL_SCANCODE_RIGHT;
            int lookUp = SDL_SCANCODE_UP;
            int lookDown = SDL_SCANCODE_DOWN;
        };

        // TODO : Ajouter une couche d'abstraction
        void moveInPlaneXZ(SDL_Window* window, float deltaTime, Core::Liara_GameObject& gameObject) const;

        KeyMappings m_KeyMappings{};
        float m_MoveSpeed = 3.0f;
        float m_LookSpeed = 1.5f;
    };
}
