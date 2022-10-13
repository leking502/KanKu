//
// Created by leking on 22-10-12.
//

#include "controller.hpp"

namespace leking {
    void Controller::moveInPlaneXZ(GLFWwindow *window, float dt, KanGameObject &gameObject) {
        glm::vec3 rotate{0};
        if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
        if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
        if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
        if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS && !onDrop) {
            glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            clickPos.x = cursorPos.x;
            clickPos.y = cursorPos.y;
            onDrop = true;
        }
        if(onDrop) {
            CursorPos newCursorPos{};
            glfwGetCursorPos(window, &newCursorPos.x, &newCursorPos.y);
            glfwSetCursorPos(window, clickPos.x,clickPos.y);
            rotate.x += lookSpeed * float(clickPos.y - newCursorPos.y ) * 0.0005f;
            rotate.y -= lookSpeed * float(clickPos.x - newCursorPos.x ) * 0.0005f;
            gameObject.transform.rotation+=rotate;
        }
        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE && onDrop) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            onDrop = false;
        }

        if (glm::dot(rotate,rotate) > std::numeric_limits<float>::epsilon()) {
            //gameObject.modelMatrix.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y,glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
        const glm::vec3 upDir{0.0f, -1.0f, 0.0f};

        glm::vec3 moveDir{0.0f};
        if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir,moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
} // leking