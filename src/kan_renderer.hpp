//
// Created by kaning on 22-10-11.
//

#ifndef KANKU_KAN_RENDERER_HPP
#define KANKU_KAN_RENDERER_HPP

#include <vulkan/vulkan_core.h>
#include "kan_device.hpp"
#include "kan_swap_chain.hpp"

namespace leking {

    class KanRenderer {
    public:

        KanRenderer(KanDevice& device, KanWindow& window);
        ~KanRenderer();

        KanRenderer(const KanRenderer &) = delete;
        KanRenderer& operator=(const KanRenderer&) = delete;

        VkFormat getSwapChainImageFormat() {return kanSwapChain->getSwapChainImageFormat();}
        size_t imageCount() { return kanSwapChain->imageCount(); }
        float getAspectRatio() const { return kanSwapChain->extentAspectRatio();}
        bool isFrameInProgress() const {return  isFrameStarted;}

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame is not in program");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "The frame index cannot be obtained when the frame is not in the program");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        std::unique_ptr<KanSwapChain>& GetKanSwapChain() {return kanSwapChain; }

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        KanWindow& kanWindow;
        KanDevice& kanDevice;
        std::unique_ptr<KanSwapChain> kanSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };

} // leking

#endif //KANKU_KAN_RENDERER_HPP
