//
// Created by kaning on 22-10-11.
//

#include <iostream>
#include <cassert>
#include "kan_renderer.hpp"
#include "vk_initializers.h"

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)


namespace leking {
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    KanRenderer::KanRenderer(KanDevice& device, KanWindow& window) : kanDevice(device), kanWindow(window){
        recreateSwapChain();
        createCommandBuffers();
    }

    KanRenderer::~KanRenderer() {
        freeCommandBuffers();
    }


    void KanRenderer::recreateSwapChain() {
        auto extent = kanWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = kanWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(kanDevice.Device());
        if(kanSwapChain == nullptr) {
            kanSwapChain = std::make_unique<KanSwapChain>(kanDevice, extent);
        } else {
            std::shared_ptr<KanSwapChain> oldSwapChain = std::move(kanSwapChain);
            kanSwapChain = std::make_unique<KanSwapChain>(kanDevice, extent, oldSwapChain);

            if(!oldSwapChain->compareSwapFormats(*kanSwapChain.get())) {
                throw std::runtime_error("交换链图像或深度格式发生变化");
            }
        }

        //如果渲染通道兼容的话可以不创建新的Pipeline
    }

    void KanRenderer::createCommandBuffers() {
        commandBuffers.resize(KanSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = kanDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t >(commandBuffers.size());

        if(vkAllocateCommandBuffers(kanDevice.Device(), &allocInfo, commandBuffers.data()) !=  VK_SUCCESS) {
            throw std::runtime_error("分配命令缓冲区失败");
        }
    }


    void KanRenderer::freeCommandBuffers() {;
        vkFreeCommandBuffers(
                kanDevice.Device(),
                kanDevice.getCommandPool(),
                commandBuffers.size(),
                commandBuffers.data());
        commandBuffers.clear();
    }


    VkCommandBuffer KanRenderer::beginFrame() {
        assert(!isFrameStarted && "不可以在帧已经在程序的情况下调用开始帧");

        auto result = kanSwapChain->acquireNextImage(&currentImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("无法获得交换链图像");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("开始记录命令缓冲区失败");
        }
        return commandBuffer;
    }

    void KanRenderer::endFrame() {
        assert(isFrameStarted && "不可以在帧不在程序的时候调用结束帧");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("记录命令缓冲区失败");
        }
        auto result = kanSwapChain->submitCommandBuffers(&commandBuffer ,&currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || kanWindow.wasWindowResized()) {
            kanWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("提交交换链图像失败");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % KanSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void KanRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "不可以在帧不在程序的时候调用开始交换链渲染通道");
        assert(commandBuffer == getCurrentCommandBuffer() &&"开始渲染不可以在不同的帧上传递给命令缓冲区");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = kanSwapChain->RenderPass();
        renderPassInfo.framebuffer = kanSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = kanSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(kanSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(kanSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, kanSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void KanRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "You cannot call the end exchange chain rendering channel when the frame is not in the program");
        assert(commandBuffer == getCurrentCommandBuffer() &&"End rendering cannot be passed to the command buffer on different frames");
        vkCmdEndRenderPass(commandBuffer);
    }
} // leking