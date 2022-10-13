
#include "vk_engine.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#define MAX_FRAME_TIME 1.0f

#include <vk_types.h>
#include <vk_initializers.h>
#include <iostream>
#include <fstream>

#include "VkBootstrap.h"
#include "pipeline_builder.hpp"
#include "kan_camera.hpp"
#include "controller.hpp"
#include "maze_game.hpp"
#include "vk_textures.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


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

void VulkanEngine::cleanup()
{
    if(isInitialized){
        for(auto f :kanRenderer->GetKanSwapChain()->GetInFlightFences()) {
            vkWaitForFences(kanDevice.Device(), 1, &f, true, 1000000000);
        }
        kanRenderer = nullptr;
        mainDeletionQueue.flush();
    }
}


void VulkanEngine::draw(float dt)
{

}
void VulkanEngine::mainLoop() {

    //对象初始化
    leking::KanCamera camera{};
    auto viewerObject = leking::KanGameObject::createGameObject();
    leking::Controller cameraController{};
    auto currentTime = std::chrono::high_resolution_clock::now();

    int mw = 10,mh = 10;

    //init_scene();

    sceneParameters.sunlightDirection = { 1.f, -3.0f, -1.0f,0};
    sceneParameters.ambientColor = {1.0f,1.0f,1.0f,0.2f};
    sceneParameters.sunlightColor = {1, 1, 1, 0.8};

    leking::MazeGameManager mazeGameManager = leking::MazeGameManager(kanDevice, gameObjects,meshes,materials,mw,mh);
    mazeGameManager.CreateMaze();

    viewerObject.transform.translation = { mw/2.f,-1.2*glm::max(mw,mh),(mh/2.f)-4.f };
    viewerObject.transform.rotation = { -glm::half_pi<float>(),0.0f,0.0f};


    while(!glfwWindowShouldClose(kanWindow.getGLFWwindow())) {
        glfwPollEvents();

        //获得dt
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        frameTime = glm::min(frameTime, MAX_FRAME_TIME);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        ImGui::SetWindowPos({0,0});

        ImGui::Text("Kanku");
        ImGui::InputInt(u8"宽",&mw);
        ImGui::InputInt(u8"高",&mh);

        //进入帧
        if(auto  commandBuffer = kanRenderer->beginFrame()) {
            //相机设置
            {
                cameraController.moveInPlaneXZ(kanWindow.getGLFWwindow(), frameTime, viewerObject);
                camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

                float aspect = kanRenderer->getAspectRatio();
                //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
                camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 1000.0f);

                GPUCameraData camData;
                camData.projectionView = camera.getProjection() * camera.getView();

                void *data;
                vmaMapMemory(kanDevice.Allocator(), cameraBuffers[kanRenderer->getFrameIndex()].allocation, &data);

                memcpy(data, &camData, sizeof(GPUCameraData));

                vmaUnmapMemory(kanDevice.Allocator(), cameraBuffers[kanRenderer->getFrameIndex()].allocation);
            }
            mazeGameManager.Update(kanWindow.getGLFWwindow(), frameTime);

            kanRenderer->beginSwapChainRenderPass(commandBuffer);
            ImGui::Render();
            //开始渲染
            drawObjects(commandBuffer, gameObjects,frameTime);

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            kanRenderer->endSwapChainRenderPass(commandBuffer);
            kanRenderer->endFrame();
            frameNumber+=1;
        }
    }
}

void VulkanEngine::init(){
    //initWindow();
    //initVulkan();
    //initSwapChain();
    //initDefaultRenderPass();
    //initFrameBuffers();
    initCommands();
    initSyncStructures();
    kanRenderer = std::make_unique<leking::KanRenderer>(kanDevice, kanWindow);
    initDescriptors();
    initPipelines();
    loadImages();
    loadMeshes();
    initImgui();
    //init_scene();
    isInitialized = true;
}

void VulkanEngine::run()
{
    init();
    mainLoop();
    cleanup();
}

bool VulkanEngine::loadShaderModule(const std::string &filePath, VkShaderModule *outShaderModule) {
    std::ifstream  file(filePath, std::ios::ate | std::ios::binary);

    if(!file.is_open()) {
        return false;
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);

    file.read((char*)buffer.data(), fileSize);

    file.close();

    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    VkShaderModule shaderModule;
    if(vkCreateShaderModule(kanDevice.Device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;
}

void VulkanEngine::initPipelines() {

    VkShaderModule texturedMeshShader;
    if (!loadShaderModule("../shaders/TexturedLit.frag.spv", &texturedMeshShader))
    {
        std::cout << "Error when building the textured mesh shader" << std::endl;
    }
    VkShaderModule meshVertShader;
    if (!loadShaderModule("../shaders/meshShader.vert.spv", &meshVertShader)) {
        std::cout << "Error when building the mesh vertex shader module" << std::endl;
    }else {
        std::cout << "Red Triangle vertex shader successfully loaded" << std::endl;
    }
    VkShaderModule colorMeshShader;
    if (!loadShaderModule("../shaders/DefaultLit.frag.spv", &colorMeshShader))
    {
        std::cout << "Error when building the colored mesh shader" << std::endl;
    }


//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
    PipelineBuilder pipelineBuilder;

    pipelineBuilder.shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

    pipelineBuilder.shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colorMeshShader));


    //we start from just the default empty pipeline layout info
    VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipelineLayoutCreateInfo();

    //setup push constants
    VkPushConstantRange push_constant;
    //offset 0
    push_constant.offset = 0;
    //size of a MeshPushConstant struct
    push_constant.size = sizeof(MeshPushConstants);
    //for the vertex shader
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
    mesh_pipeline_layout_info.pushConstantRangeCount = 1;

    VkDescriptorSetLayout setLayouts[] = { globalSetLayout, objectSetLayout };

    mesh_pipeline_layout_info.setLayoutCount = 2;
    mesh_pipeline_layout_info.pSetLayouts = setLayouts;

    VkPipelineLayout meshPipLayout;
    VK_CHECK(vkCreatePipelineLayout(kanDevice.Device(), &mesh_pipeline_layout_info, nullptr, &meshPipLayout));


    //we start from  the normal mesh layout
    VkPipelineLayoutCreateInfo textured_pipeline_layout_info = mesh_pipeline_layout_info;

    VkDescriptorSetLayout texturedSetLayouts[] = { globalSetLayout, objectSetLayout,singleTextureSetLayout };

    textured_pipeline_layout_info.setLayoutCount = 3;
    textured_pipeline_layout_info.pSetLayouts = texturedSetLayouts;

    VkPipelineLayout texturedPipeLayout;
    VK_CHECK(vkCreatePipelineLayout(kanDevice.Device(), &textured_pipeline_layout_info, nullptr, &texturedPipeLayout));

    //hook the push constants layout
    pipelineBuilder.pipelineLayout = meshPipLayout;

    //vertex input controls how to read vertices from vertex buffers. We arent using it yet
    pipelineBuilder.vertexInputInfo = vkinit::vertexInputStateCreateInfo();

    //input assembly is the configuration for drawing triangle lists, strips, or individual points.
    //we are just going to draw triangle list
    pipelineBuilder.inputAssembly = vkinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);


    //configure the rasterizer to draw filled triangles
    pipelineBuilder.rasterizer = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    //we dont use multisampling, so just run the default one
    pipelineBuilder.multisampling = vkinit::multisamplingStateCreateInfo();

    //a single blend attachment with no blending and writing to RGBA
    pipelineBuilder.colorBlendAttachment = vkinit::colorBlendAttachmentState();


    //default depthtesting
    pipelineBuilder.depthStencil = vkinit::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    //build the mesh pipeline

    VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();


    //build the mesh triangle pipeline
    VkPipeline meshPipeline = pipelineBuilder.buildPipeline(kanDevice.Device(), kanRenderer->GetKanSwapChain()->RenderPass());

    createMaterial(meshPipeline, meshPipLayout, "defaultMesh");

    pipelineBuilder.shaderStages.clear();
    pipelineBuilder.shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

    pipelineBuilder.shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, texturedMeshShader));

    pipelineBuilder.pipelineLayout = texturedPipeLayout;
    VkPipeline texPipeline = pipelineBuilder.buildPipeline(kanDevice.Device(), kanRenderer->GetKanSwapChain()->RenderPass());
    createMaterial(texPipeline, texturedPipeLayout, "texturedMesh");


    vkDestroyShaderModule(kanDevice.Device(), meshVertShader, nullptr);
    vkDestroyShaderModule(kanDevice.Device(), colorMeshShader, nullptr);
    vkDestroyShaderModule(kanDevice.Device(), texturedMeshShader, nullptr);


    mainDeletionQueue.push([=]() {
        vkDestroyPipeline(kanDevice.Device(), meshPipeline, nullptr);
        vkDestroyPipeline(kanDevice.Device(), texPipeline, nullptr);

        vkDestroyPipelineLayout(kanDevice.Device(), meshPipLayout, nullptr);
        vkDestroyPipelineLayout(kanDevice.Device(), texturedPipeLayout, nullptr);
    });
}

void VulkanEngine::loadMeshes() {
    //make the array 3 vertices long
    Mesh triangleMesh, monkeyMesh, cubeMesh, lostEmpire;
    triangleMesh.vertices.resize(3);

    //vertex positions
    triangleMesh.vertices[0].position = { 1.f, 1.f, 0.0f };
    triangleMesh.vertices[1].position = {-1.f, 1.f, 0.0f };
    triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

    //vertex colors, all green
    triangleMesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
    triangleMesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
    triangleMesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

    monkeyMesh.loadFromObj("../assets/monkey_smooth.obj");
    cubeMesh.loadFromObj("../assets/cube.obj");
    lostEmpire.loadFromObj("../assets/lost_empire.obj");

    //we don't care about the vertex normals

    uploadMesh(triangleMesh);
    uploadMesh(monkeyMesh);
    uploadMesh(cubeMesh);
    uploadMesh(lostEmpire);

    meshes["monkey"] = std::make_shared<Mesh>(monkeyMesh);
    meshes["triangle"] = std::make_shared<Mesh>(triangleMesh);
    meshes["cube"] = std::make_shared<Mesh>(cubeMesh);
    meshes["empire"] = std::make_shared<Mesh>(lostEmpire);
}

void VulkanEngine::uploadMesh(Mesh &mesh) {

    const size_t bufferSize= mesh.vertices.size() * sizeof(Vertex);
    //allocate staging buffer
    VkBufferCreateInfo stagingBufferInfo = {};
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.pNext = nullptr;

    stagingBufferInfo.size = bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    //let the VMA library know that this data should be on CPU RAM
    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    AllocatedBuffer stagingBuffer{};

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(kanDevice.Allocator(), &stagingBufferInfo, &vmaAllocInfo,
                             &stagingBuffer.buffer,
                             &stagingBuffer.allocation,
                             nullptr));

    void* data;
    vmaMapMemory(kanDevice.Allocator(), stagingBuffer.allocation, &data);

    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(kanDevice.Allocator(), stagingBuffer.allocation);

    //allocate vertex buffer
    VkBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.pNext = nullptr;
    //this is the total size, in bytes, of the buffer we are allocating
    vertexBufferInfo.size = bufferSize;
    //this buffer is going to be used as a Vertex Buffer
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    //let the VMA library know that this data should be GPU native
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(kanDevice.Allocator(), &vertexBufferInfo, &vmaAllocInfo,
                             &mesh.vertexBuffer.buffer,
                             &mesh.vertexBuffer.allocation,
                             nullptr));

    immediateSubmit([=](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = bufferSize;
        vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, &copy);
    });

    mainDeletionQueue.push([=]() {
        vmaDestroyBuffer(kanDevice.Allocator(), mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
    });

    vmaDestroyBuffer(kanDevice.Allocator(), stagingBuffer.buffer, stagingBuffer.allocation);

}

std::shared_ptr<leking::Material> VulkanEngine::createMaterial(VkPipeline& pipeline, VkPipelineLayout& layout, const std::string &name) {
    std::shared_ptr<leking::Material> mat = std::make_shared<leking::Material>(pipeline,layout);
    materials[name] = mat;
    return materials[name];
}

std::shared_ptr<leking::Material> VulkanEngine::getMaterial(const std::string &name) {
    //search for the object, and return nullptr if not found
    auto it = materials.find(name);
    if (it == materials.end()) {
        return nullptr;
    }
    else {
        return (*it).second;
    }
}

std::shared_ptr<Mesh> VulkanEngine::getMesh(const std::string &name) {
    auto it = meshes.find(name);
    if (it == meshes.end()) {
        return nullptr;
    }
    else {
        return (*it).second;
    }
}

void VulkanEngine::drawObjects(VkCommandBuffer cmd,std::vector<leking::KanGameObject>& _gameObjects,float dt) {

    char* sceneData;
    vmaMapMemory(kanDevice.Allocator(), sceneParameterBuffer.allocation , (void**)&sceneData);

    int frameIndex = kanRenderer->getFrameIndex();

    sceneData += padUniformBufferSize(sizeof(GPUSceneData)) * frameIndex;

    memcpy(sceneData, &sceneParameters, sizeof(GPUSceneData));

    vmaUnmapMemory(kanDevice.Allocator(), sceneParameterBuffer.allocation);

    void* objectData;
    vmaMapMemory(kanDevice.Allocator(), objectBuffers[kanRenderer->getFrameIndex()].allocation, &objectData);

    auto* objectSSBO = (GPUObjectData*)objectData;

    for(int i = 0; i<gameObjects.size();i++){
        leking::KanGameObject& object = gameObjects[i];
        objectSSBO[i].modelMatrix = object.transform.mat4();
        objectSSBO[i].normalMatrix = object.transform.normalMatrix();
    }
    vmaUnmapMemory(kanDevice.Allocator(), objectBuffers[kanRenderer->getFrameIndex()].allocation);

    std::shared_ptr<Mesh> lastMesh = nullptr;
    std::shared_ptr<leking::Material> lastMaterial = nullptr;
    int i = 0;
    for(auto& object : gameObjects){
        if(!object.CanDraw()){
            i+=1;
            continue;
        }
        //only bind the pipeline if it doesn't match with the already bound one
        if (object.material != lastMaterial) {

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;
            //offset for our scene buffer
            uint32_t uniformOffset = padUniformBufferSize(sizeof(GPUSceneData)) * kanRenderer->getFrameIndex();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &globalDescriptors[kanRenderer->getFrameIndex()], 1, &uniformOffset);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &objectDescriptors[kanRenderer->getFrameIndex()], 0, nullptr);
            if(object.material->textureSet!=VK_NULL_HANDLE)
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 2, 1, &object.material->textureSet, 0, nullptr);

        }

        glm::mat4 model = object.transform.mat4();
        //final render matrix, that we are calculating on the cpu
        glm::mat4 mesh_matrix = model;

        MeshPushConstants constants;
        constants.color = glm::vec4{object.color,0.0f};

        //upload the mesh to the GPU via push constants
        vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

        //only bind the mesh if it's a different one from last bind
        if (object.mesh != lastMesh) {
            //bind the mesh vertex buffer with offset 0
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
            lastMesh = object.mesh;
        }
        //we can now draw
        vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, i);
        i+=1;
    }
}

void VulkanEngine::init_scene() {
    leking::KanGameObject map = leking::KanGameObject::createGameObject();
    map.mesh = getMesh("empire");

    VkSamplerCreateInfo samplerInfo = vkinit::samplerCreateInfo(VK_FILTER_NEAREST);

    VkSampler blockSampler;
    vkCreateSampler(kanDevice.Device(), &samplerInfo, nullptr, &blockSampler);
    mainDeletionQueue.push([=]{
        vkDestroySampler(kanDevice.Device(),blockSampler, nullptr);
    });

    std::shared_ptr<leking::Material> texturedMat = getMaterial("texturedMesh");
    map.material = texturedMat;

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &singleTextureSetLayout;

    vkAllocateDescriptorSets(kanDevice.Device(), &allocInfo, &texturedMat->textureSet);

    //write to the descriptor set so that it points to our empire_diffuse texture
    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler = blockSampler;
    imageBufferInfo.imageView = loadedTextures["empire_diffuse"].imageView;
    imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet texture1 = vkinit::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);

    vkUpdateDescriptorSets(kanDevice.Device(), 1, &texture1, 0, nullptr);

    map.transform.translation = glm::vec3{ 5,15,0 };
    map.transform.rotation = {glm::pi<float>(),0,0};

    gameObjects.push_back(std::move(map));
}

void VulkanEngine::initDescriptors() {
    //create a descriptor pool that will hold 10 uniform buffers
    std::vector<VkDescriptorPoolSize> sizes =
            {
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 10;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    vkCreateDescriptorPool(kanDevice.Device(), &pool_info, nullptr, &descriptorPool);

    //information about the binding.
    VkDescriptorSetLayoutBinding camBufferBinding = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);

    VkDescriptorSetLayoutBinding sceneBinding= vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

    VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

    VkDescriptorSetLayoutBinding bindings[] = { camBufferBinding,sceneBinding };


    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pNext = nullptr;
    setinfo.bindingCount = 2;
    setinfo.flags = 0;
    setinfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(kanDevice.Device(), &setinfo, nullptr, &globalSetLayout);

    VkDescriptorSetLayoutCreateInfo set2info = {};
    set2info.bindingCount = 1;
    set2info.flags = 0;
    set2info.pNext = nullptr;
    set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2info.pBindings = &objectBind;

    vkCreateDescriptorSetLayout(kanDevice.Device(), &set2info, nullptr, &objectSetLayout);

    VkDescriptorSetLayoutCreateInfo set3info = {};
    set3info.bindingCount = 1;
    set3info.flags = 0;
    set3info.pNext = nullptr;
    set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set3info.pBindings = &textureBind;

    vkCreateDescriptorSetLayout(kanDevice.Device(), &set3info, nullptr, &singleTextureSetLayout);

    const size_t sceneParamBufferSize = leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT * padUniformBufferSize(sizeof(GPUSceneData));
    sceneParameterBuffer = createBuffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    for (int i = 0; i < leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT; i++){

        const int MAX_OBJECTS = 100000;
        objectBuffers[i] = createBuffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        cameraBuffers[i] = createBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        //allocate one descriptor set for each frame
        VkDescriptorSetAllocateInfo allocInfo ={};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //using the pool we just set
        allocInfo.descriptorPool = descriptorPool;
        //only 1 descriptor
        allocInfo.descriptorSetCount = 1;
        //using the global data layout
        allocInfo.pSetLayouts = &globalSetLayout;

        vkAllocateDescriptorSets(kanDevice.Device(), &allocInfo, &globalDescriptors[i]);

        VkDescriptorSetAllocateInfo objectSetAlloc = {};
        objectSetAlloc.pNext = nullptr;
        objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        objectSetAlloc.descriptorPool = descriptorPool;
        objectSetAlloc.descriptorSetCount = 1;
        objectSetAlloc.pSetLayouts = &objectSetLayout;

        vkAllocateDescriptorSets(kanDevice.Device(), &objectSetAlloc, &objectDescriptors[i]);


        //information about the buffer we want to point at in the descriptor
        VkDescriptorBufferInfo cameraInfo;
        cameraInfo.buffer = cameraBuffers[i].buffer;
        cameraInfo.offset = 0;
        cameraInfo.range = sizeof(GPUCameraData);

        VkDescriptorBufferInfo sceneInfo;
        sceneInfo.buffer = sceneParameterBuffer.buffer;
        sceneInfo.offset = 0;
        sceneInfo.range = sizeof(GPUSceneData);

        VkDescriptorBufferInfo objectBufferInfo;
        objectBufferInfo.buffer = objectBuffers[i].buffer;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

        VkWriteDescriptorSet cameraWrite = vkinit::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, globalDescriptors[i],&cameraInfo,0);

        VkWriteDescriptorSet sceneWrite = vkinit::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, globalDescriptors[i], &sceneInfo, 1);

        VkWriteDescriptorSet objectWrite = vkinit::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectDescriptors[i], &objectBufferInfo, 0);


        VkWriteDescriptorSet setWrites[] = { cameraWrite,sceneWrite,objectWrite };

        vkUpdateDescriptorSets(kanDevice.Device(), 3, setWrites, 0, nullptr);
    }
    mainDeletionQueue.push([&]() {
        // other code ....
        vmaDestroyBuffer(kanDevice.Allocator(), sceneParameterBuffer.buffer, sceneParameterBuffer.allocation);

        vkDestroyDescriptorSetLayout(kanDevice.Device(), globalSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(kanDevice.Device(), objectSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(kanDevice.Device(), singleTextureSetLayout, nullptr);
        vkDestroyDescriptorPool(kanDevice.Device(), descriptorPool, nullptr);

        for (int i = 0; i < leking::KanSwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            vmaDestroyBuffer(kanDevice.Allocator(), cameraBuffers[i].buffer, cameraBuffers[i].allocation);
            vmaDestroyBuffer(kanDevice.Allocator(), objectBuffers[i].buffer, objectBuffers[i].allocation);
        }
    });
}

AllocatedBuffer VulkanEngine::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    //allocate vertex buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;


    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    AllocatedBuffer newBuffer{};

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(kanDevice.Allocator(), &bufferInfo, &vmaallocInfo,
                             &newBuffer.buffer,
                             &newBuffer.allocation,
                             nullptr));

    return newBuffer;
}
size_t VulkanEngine::padUniformBufferSize(size_t originalSize)
{
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = kanDevice.properties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}

void VulkanEngine::initSyncStructures() {
    VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fenceCreateInfo();

    VK_CHECK(vkCreateFence(kanDevice.Device(), &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence));
    mainDeletionQueue.push([=]() {
        vkDestroyFence(kanDevice.Device(), uploadContext.uploadFence, nullptr);
    });
}

void VulkanEngine::initCommands() {
    VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::commandPoolCreateInfo(kanDevice.findPhysicalQueueFamilies().graphicsFamily);
    //create pool for upload context
    VK_CHECK(vkCreateCommandPool(kanDevice.Device(), &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));

    mainDeletionQueue.push([=]() {
        vkDestroyCommandPool(kanDevice.Device(), uploadContext.commandPool, nullptr);
    });

    //allocate the default command buffer that we will use for the instant commands
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::commandBufferAllocateInfo(uploadContext.commandPool, 1);

    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(kanDevice.Device(), &cmdAllocInfo, &uploadContext.commandBuffer));
}

void VulkanEngine::immediateSubmit(std::function<void(VkCommandBuffer)> &&function) {
    VkCommandBuffer cmd = uploadContext.commandBuffer;

    //begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //execute the function
    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = vkinit::submitInfo(&cmd);


    //submit command buffer to the queue and execute it.
    // _uploadFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(kanDevice.graphicsQueue(), 1, &submit, uploadContext.uploadFence));

    vkWaitForFences(kanDevice.Device(), 1, &uploadContext.uploadFence, true, 9999999999);
    vkResetFences(kanDevice.Device(), 1, &uploadContext.uploadFence);

    // reset the command buffers inside the command pool
    vkResetCommandPool(kanDevice.Device(), uploadContext.commandPool, 0);
}

void VulkanEngine::loadImages() {
    Texture lostEmpire;

    vkUtil::loadImageFromFile(*this, "../assets/lost_empire-RGBA.png", lostEmpire.image);

    VkImageViewCreateInfo imageInfo = vkinit::imageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, lostEmpire.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
    vkCreateImageView(kanDevice.Device(), &imageInfo, nullptr, &lostEmpire.imageView);
    mainDeletionQueue.push([=]{
        vkDestroyImageView(kanDevice.Device(),lostEmpire.imageView, nullptr);
    });

    loadedTextures["empire_diffuse"] = lostEmpire;
}
void VulkanEngine::initImgui()
{
    //1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    VkDescriptorPoolSize pool_sizes[] =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(kanDevice.Device(), &pool_info, nullptr, &imguiPool));


    // 2: initialize imgui library

    //this initializes the core structures of imgui
    ImGui::CreateContext();

    //this initializes imgui for SDL
    ImGui_ImplGlfw_InitForVulkan(kanWindow.getGLFWwindow(), true);

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = kanDevice.Instance();
    init_info.PhysicalDevice = kanDevice.PhysicalDevice();
    init_info.Device = kanDevice.Device();
    init_info.Queue = kanDevice.graphicsQueue();
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, kanRenderer->GetKanSwapChain()->RenderPass());

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../Fonts/dc.ttf", 30.f, nullptr,io.Fonts->GetGlyphRangesChineseFull());

    //execute a gpu command to upload imgui font textures
    immediateSubmit([&](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    //add the destroy the imgui created structures
    mainDeletionQueue.push([=]() {

        vkDestroyDescriptorPool(kanDevice.Device(), imguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
    });
}




