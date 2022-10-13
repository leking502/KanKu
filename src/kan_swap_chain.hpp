//
// Created by Kaning on 22-10-10.
//

#ifndef KANKU_KAN_SWAP_CHAIN_HPP
#define KANKU_KAN_SWAP_CHAIN_HPP

#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
#include "kan_device.hpp"
#include "deletion_manager.hpp"
#include "vk_types.h"

namespace leking {
    
    class KanSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        KanSwapChain(KanDevice &deviceRef, VkExtent2D windowExtent);

        KanSwapChain(KanDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<KanSwapChain> previous);

        ~KanSwapChain();

        KanSwapChain(const KanSwapChain &) = delete;

        KanSwapChain &operator=(const KanSwapChain &) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }

        VkRenderPass RenderPass() { return renderPass; }

        VkImageView getImageView(int index) { return swapChainImageViews[index]; }

        size_t imageCount() { return swapChainImages.size(); }

        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }

        VkExtent2D getSwapChainExtent() { return swapChainExtent; }

        uint32_t width() { return swapChainExtent.width; }

        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t *imageIndex);

        VkFormat swapChainDepthFormat;

        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool compareSwapFormats(const KanSwapChain &swapChain) const {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat;
        }
        std::vector<VkFramebuffer>& FrameBuffers() {return swapChainFramebuffers;}

        VkExtent2D& SwapChainExtent(){ return swapChainExtent;}

        std::vector<VkFence>& GetInFlightFences() {return imagesInFlight;};

    private:
        void init();

        void createSwapChain();

        void createImageViews();

        void createDepthResources();

        void createRenderPass();

        void createFrameBuffers();

        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
                const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(
                const std::vector<VkPresentModeKHR> &availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        KanDevice &device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<KanSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };
} // leking

#endif //KANKU_KAN_SWAP_CHAIN_HPP
