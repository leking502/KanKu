//
// Created by leking on 22-10-9.
//

#ifndef KANKU_VK_MESH_HPP
#define KANKU_VK_MESH_HPP

#include <vector>
#include "glm/vec3.hpp"
#include "vk_types.h"
#include "glm/vec2.hpp"

struct VertexInputDescription {

    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;

    static VertexInputDescription getVertexDescription();
};

struct Mesh {
    std::vector<Vertex> vertices;

    AllocatedBuffer vertexBuffer;
    bool loadFromObj(const char* filename);
};


#endif //KANKU_VK_MESH_HPP
