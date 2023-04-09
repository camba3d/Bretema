#pragma once

#include "../btm_base.hpp"
#include "vk_base.hpp"

#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

namespace btm::vk
{

struct AllocatedBuffer
{
    VkBuffer      buffer     = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

struct AllocatedImage
{
    VkImage       image      = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

struct VertexInputDescription
{
    std::vector<VkVertexInputBindingDescription>   bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;

    static VertexInputDescription const &get()
    {
        static auto const desc = []()
        {
            VertexInputDescription D;
            D.bindings.push_back({ 0, sizeof(float) * (3 + 2 + 3 + 4), VK_VERTEX_INPUT_RATE_VERTEX });
            D.attributes.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });     // pos
            D.attributes.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT, 0 });        // uv0
            D.attributes.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });     // normal
            D.attributes.push_back({ 3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 });  // tangent
            return D;
        }();

        return desc;
    }
};

struct Mesh
{
    u32             indexCount = 0;
    AllocatedBuffer indices    = {};
    AllocatedBuffer vertices   = {};

    inline void draw(VkCommandBuffer cmd) const
    {
        // ROOM TO IMPROVEMENT : https://developer.nvidia.com/vulkan-memory-management

        /* @DANI
         You should store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in
         commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because
         it's closer together. It is even possible to reuse the same chunk of memory for multiple resources if they are not
         used during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
         and some Vulkan functions have explicit flags to specify that you want to do this.
        */

        VkBuffer const     v[]       = { vertices.buffer };
        VkDeviceSize const offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, v, offsets);
        vkCmdBindIndexBuffer(cmd, indices.buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
    }
};
using MeshGroup = std::vector<Mesh>;

struct Queue
{
    Queue() = default;

    Queue(vkb::Device vkbDevice, vkb::QueueType queueType)
    {
        auto const q = vkbDevice.get_queue(queueType);
        auto const f = vkbDevice.get_queue_index(queueType);
        if (valid = q.has_value() && f.has_value(); valid)
        {
            queue  = q.value();
            family = f.value();
        }
        BTM_ASSERT(valid);
    }

    VkQueue queue  = {};
    u32     family = {};
    bool    valid  = false;
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
    VkPipelineDepthStencilStateCreateInfo        depthStencil;
};

u32 constexpr RGBA_BIT = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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

template<>
struct fmt::formatter<btm::vk::Queue>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(btm::vk::Queue const &q, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const s = q.valid ? fmt::to_string(q.family) : "x";
        return fmt::format_to(ctx.out(), "{}", s);
    }
};