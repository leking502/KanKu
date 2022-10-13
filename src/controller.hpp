//
// Created by leking on 22-10-12.
//

#ifndef KANKU_CONTROLLER_HPP
#define KANKU_CONTROLLER_HPP

#include <GLFW/glfw3.h>
#include "kan_game_object.hpp"

namespace leking {

    struct CursorPos {
        double x;
        double y;
    };
    class Controller {

    public:
        struct KeyMapping {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_SHIFT;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void moveInPlaneXZ(GLFWwindow* window, float dt, KanGameObject& gameObject);

        KeyMapping keys{};
        float moveSpeed{3.0f};
        float lookSpeed{1.5f};
    private:

        CursorPos clickPos{0};
        CursorPos cursorPos{0};
        bool onDrop{false};
    };

} // leking

#endif //KANKU_CONTROLLER_HPP
