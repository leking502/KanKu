// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <deque>
#include <functional>
#include "vk_mem_alloc.h"
#include "vk_mesh.hpp"
#include "kan_window.hpp"
#include "kan_device.hpp"
#include <glm/glm.hpp>
#include "deletion_manager.hpp"
#include "kan_game_object.hpp"
#include "kan_renderer.hpp"

struct MeshPushConstants {
    glm::mat4 modelMatrix{1.0f};
    glm::vec4 color{0.0f};
};
struct GPUCameraData{
    alignas(16) glm::mat4 projectionView{1.f};
};
struct GPUSceneData {
    glm::vec4 fogColor; // w is for exponent
    glm::vec4 fogDistances; //x for min, y for max, zw unused.
    glm::vec4 ambientColor;
    glm::vec4 sunlightDirection{1.0f}; //w for sun power
    glm::vec4 sunlightColor;
};
struct GPUObjectData{
    glm::mat4 modelMatrix{1.0f};
    glm::mat4 normalMatrix{1.0f};
};
struct UploadContext {
    VkFence uploadFence;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
};
struct Texture {
    AllocatedImage image;
    VkImageView imageView;
};

class VulkanEngine {
public:

	bool isInitialized{ false };
	int frameNumber {0};
    int selectedShader{ 0 };

    bool isKeyDown{false};

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
        int changeShader = GLFW_KEY_SPACE;
    };

    KeyMapping keys{};

    static constexpr int WIDTH = 1600;
    static constexpr int HEIGHT = 1600;

    //deletion
    DeletionQueue mainDeletionQueue;

    leking::KanWindow kanWindow {WIDTH, HEIGHT, "LekEngine"};
    leking::KanDevice kanDevice {kanWindow};
    std::unique_ptr<leking::KanRenderer> kanRenderer;

   // VkCommandBuffer mainCommandBuffer;

    std::vector<leking::KanGameObject> gameObjects;

    GPUSceneData sceneParameters;
    AllocatedBuffer sceneParameterBuffer;
    AllocatedBuffer cameraBuffers[leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT];
    AllocatedBuffer objectBuffers[leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSet globalDescriptors[leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSet objectDescriptors[leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT];

    VkDescriptorSetLayout globalSetLayout;
    VkDescriptorSetLayout objectSetLayout;
    VkDescriptorPool descriptorPool;

    UploadContext uploadContext;

    void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);


    std::unordered_map<std::string,std::shared_ptr<leking::Material>> materials;
    std::unordered_map<std::string,std::shared_ptr<Mesh>> meshes;
    std::unordered_map<std::string, Texture> loadedTextures;

    VkDescriptorSetLayout singleTextureSetLayout;

    void loadImages();

    std::shared_ptr<leking::Material> createMaterial(VkPipeline& pipeline, VkPipelineLayout& layout,const std::string& name);
    std::shared_ptr<leking::Material> getMaterial(const std::string& name);
    std::shared_ptr<Mesh> getMesh(const std::string& name);

    void drawObjects(VkCommandBuffer cmd,std::vector<leking::KanGameObject>& _gameObjects,float dt);

//run main loop
    void run();

    bool loadShaderModule(const std::string& filePath, VkShaderModule* outShaderModule);

    AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

private:
    void init_scene();

    bool frameBufferResized{false};
    //shuts down the engine
    void cleanup();

    //draw loop
    void draw(float dt);

    void mainLoop();

    void init();

    void loadMeshes();

    void uploadMesh(Mesh& mesh);

    void initDescriptors();

    void initPipelines();

    size_t padUniformBufferSize(size_t originalSize);

    void initSyncStructures();

    void initCommands();

    void initImgui();
};
