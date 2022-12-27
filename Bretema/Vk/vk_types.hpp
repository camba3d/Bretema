#pragma once

#include "../btm_base.hpp"
#include "vk_base.hpp"

#include <vector>

namespace btm::vk::types
{

struct Queue
{
    Queue() = default;

    Queue(vkb::Device vkbDevice, vkb::QueueType queueType)
    {
        auto q = vkbDevice.get_queue(queueType);
        auto f = vkbDevice.get_queue_index(queueType);

        if (valid = q.has_value() && f.has_value(); valid)
        {
            queue  = q.value();
            family = f.value();
        }

        BTM_ASSERT(valid);
    }

    VkQueue  queue  = {};
    uint32_t family = {};
    bool     valid  = false;
};  // namespace btm::vk::types

struct PipelineBuilder
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo         vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo       inputAssembly;
    VkViewport                                   viewport;
    VkRect2D                                     scissor;
    VkPipelineRasterizationStateCreateInfo       rasterizer;
    VkPipelineColorBlendAttachmentState          colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo         multisampling;
    VkPipelineLayout                             pipelineLayout;

    VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};

}  // namespace btm::vk::types