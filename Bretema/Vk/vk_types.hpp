#pragma once

#include "../btm_base.hpp"
#include "vk_base.hpp"

#include <vma/vk_mem_alloc.h>

#include <vector>
#include <array>

namespace btm::vk
{

//-----------------------------------------------------------------------------

u32 constexpr R_BIT    = VK_COLOR_COMPONENT_R_BIT;
u32 constexpr G_BIT    = VK_COLOR_COMPONENT_G_BIT;
u32 constexpr B_BIT    = VK_COLOR_COMPONENT_B_BIT;
u32 constexpr A_BIT    = VK_COLOR_COMPONENT_A_BIT;
u32 constexpr RGB_BIT  = R_BIT | G_BIT | B_BIT;
u32 constexpr RGBA_BIT = R_BIT | G_BIT | B_BIT | A_BIT;

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

struct AllocatedBuffer
{
    VkBuffer      buffer     = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

//-----------------------------------------------------------------------------

struct AllocatedImage
{
    VkImage       image      = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

struct Queue
{
    Queue() = default;
    Queue(vkb::Device vkbDevice, vkb::QueueType aType)
    {
        type         = aType;
        auto const q = vkbDevice.get_queue(aType);
        auto const f = vkbDevice.get_queue_index(aType);
        if (valid = q.has_value() && f.has_value(); valid)
        {
            queue  = q.value();
            family = f.value();
        }
        BTM_ASSERT(valid);
    }

    VkQueue        queue  = {};
    u32            family = {};
    vkb::QueueType type   = vkb::QueueType::graphics;
    bool           valid  = false;
};

//-----------------------------------------------------------------------------

struct QueueCmd
{
    QueueCmd() = default;
    QueueCmd(sPtr<Queue> q) : queue(q) {}

    sPtr<Queue>     queue = {};
    VkCommandPool   pool  = {};
    VkCommandBuffer cmd   = {};
};

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

struct Mesh
{
    u32             indexCount = 0;
    AllocatedBuffer indices    = {};
    AllocatedBuffer vertices   = {};

    // ROOM TO IMPROVEMENT : https://developer.nvidia.com/vulkan-memory-management

    /* @DANI
     You should store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in
     commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because
     it's closer together. It is even possible to reuse the same chunk of memory for multiple resources if they are not
     used during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
     and some Vulkan functions have explicit flags to specify that you want to do this.
    */

    inline void bind(VkCommandBuffer cmd) const
    {
        VkBuffer const     v[]       = { vertices.buffer };
        VkDeviceSize const offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, v, offsets);
        vkCmdBindIndexBuffer(cmd, indices.buffer, 0, VK_INDEX_TYPE_UINT16);
    }

    inline void draw(VkCommandBuffer cmd) const
    {  //
        vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
    }

    inline void bindNdraw(VkCommandBuffer cmd) const
    {
        bind(cmd);
        draw(cmd);
    }
};

using MeshGroup = std::vector<Mesh>;

//-----------------------------------------------------------------------------

struct Material
{
    VkPipeline       pipeline;
    VkPipelineLayout pipelineLayout;

    inline void bind(VkCommandBuffer cmd) { vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline); }
};

//-----------------------------------------------------------------------------

struct RenderObject
{
    Mesh     *mesh;
    Material *material;
    glm::mat4 transform;
};

//-----------------------------------------------------------------------------

struct Matrices
{
    glm::mat4 N;
    glm::mat4 MVP;
};

//-----------------------------------------------------------------------------

struct FrameData
{
    // * room to improve: https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation

    VkSemaphore presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore  = VK_NULL_HANDLE;
    VkFence     renderFence      = VK_NULL_HANDLE;

    vk::QueueCmd graphics = {};
    vk::QueueCmd present  = {};
    vk::QueueCmd compute  = {};
    vk::QueueCmd transfer = {};
};

//-----------------------------------------------------------------------------

}  // namespace btm::vk

//=====================================
//=== FMT
//=====================================

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