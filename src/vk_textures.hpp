//
// Created by leking on 22-10-13.
//

#pragma once

#include <vk_types.h>
#include <vk_engine.h>


namespace vkUtil {

    bool loadImageFromFile(VulkanEngine& engine, const char* file, AllocatedImage& outImage);

}

