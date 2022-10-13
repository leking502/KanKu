//
// Created by leking on 22-10-9.
//

#include <stdexcept>
#include <iostream>
#include "kan_window.hpp"

namespace leking{

    KanWindow::KanWindow(int width, int height, std::string name) : width(width), height(height), windowName(name){
        initWindow();
    }

    void KanWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);

        window = glfwCreateWindow(width,height,windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    }

    KanWindow::~KanWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool KanWindow::shouldClose() {
        return glfwWindowShouldClose(window);
    }

    void KanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("Window creation failed");
        }
    }

    VkExtent2D KanWindow::getExtent() {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    void KanWindow::frameBufferResizeCallback(GLFWwindow *window, int wight, int height) {
        auto lekWindow = reinterpret_cast<KanWindow*>(glfwGetWindowUserPointer(window));
        lekWindow->frameBufferResized = true;
        lekWindow->width = wight;
        lekWindow->height = height;
    }

}
