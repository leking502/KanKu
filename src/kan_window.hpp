//
// Created by Kaning on 22-10-9.
//

#ifndef KANKU_KAN_WINDOW_HPP
#define KANKU_KAN_WINDOW_HPP

#include <string>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace leking {

    class KanWindow{
    public:
        KanWindow(int width, int height, std::string name);
        ~KanWindow();

        KanWindow(const KanWindow &) = delete;
        KanWindow& operator=(const KanWindow&) = delete;

        bool shouldClose();
        VkExtent2D getExtent();
        bool wasWindowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        GLFWwindow* getGLFWwindow() const {return window;}

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void frameBufferResizeCallback(GLFWwindow* window, int wight, int height);
        void initWindow();
        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowName;
        GLFWwindow* window;
    };

} // Kaning

#endif //KANKU_KAN_WINDOW_HPP
