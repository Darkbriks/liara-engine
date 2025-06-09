#pragma once
#include <SDL2/SDL.h>
#include "Core/Liara_GameObject.h"
#include "Core/Liara_SettingsManager.h"

namespace Liara::Listener
{
    /**
     * @class KeybordMovementController
     * @brief Class that encapsulates keyboard movement controls.
     */
    class KeybordMovementController
    {
    public:
        /**
         * @struct KeyMappings
         * @brief Structure that encapsulates key mappings for movement and looking.
         */
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

        explicit KeybordMovementController(Core::SettingsManager& settingsManager) : m_SettingsManager(settingsManager) {}

        // TODO : Ajouter une couche d'abstraction
        /**
         * @brief Moves the game object in the XZ plane.
         * @param window The SDL window.
         * @param deltaTime The time since the last frame.
         * @param gameObject The game object to move.
         */
        void moveInPlaneXZ(SDL_Window* window, float deltaTime, Core::Liara_GameObject& gameObject) const;

        KeyMappings m_KeyMappings{};        ///< The key mappings for movement and looking
        float m_MoveSpeed = 3.0f;           ///< The movement speed
        float m_LookSpeed = 1.5f;           ///< The look speed

    private:
        Core::SettingsManager& m_SettingsManager;

        mutable bool m_F10Pressed = false;
        mutable bool m_F11Pressed = false;
    };
}
