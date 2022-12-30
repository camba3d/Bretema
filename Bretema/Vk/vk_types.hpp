#pragma once

#include "../btm_base.hpp"
#include "vk_base.hpp"

#include <vector>

namespace btm::vk
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
};

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
};

uint32_t constexpr RGBA_BIT =
  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

namespace Blend
{
VkPipelineColorBlendAttachmentState const None = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = RGBA_BIT,
};
VkPipelineColorBlendAttachmentState const Additive = {
    .blendEnable         = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = RGBA_BIT,
};
VkPipelineColorBlendAttachmentState const StraightAlpha = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = RGBA_BIT,
};
VkPipelineColorBlendAttachmentState const StraightColor = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = RGBA_BIT,
};
}  // namespace Blend

}  // namespace btm::vk