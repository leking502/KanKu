//
// Created by leking on 22-10-9.
//

#ifndef KANKU_PIPELINE_BUILDER_HPP
#define KANKU_PIPELINE_BUILDER_HPP


#include <vector>
#include <vulkan/vulkan.h>

class PipelineBuilder {
public:
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;

    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    VkPipeline buildPipeline(VkDevice device, VkRenderPass pass);
};


#endif //KANKU_PIPELINE_BUILDER_HPP
