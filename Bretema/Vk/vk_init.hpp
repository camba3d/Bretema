#pragma once

#include "../btm_base.hpp"
#include "../btm_tools.hpp"

#include "vk_base.hpp"
#include "vk_types.hpp"

#include <optional>
#include <vector>

namespace btm::vk
{

namespace CreateInfo
{

inline auto CommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0, void *pNext = nullptr)
{
    VkCommandPoolCreateInfo info {};
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext            = pNext;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags            = flags;
    return info;
}

inline auto Fence(VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT, void *pNext = nullptr)
{
    VkFenceCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = pNext;
    info.flags = flags;
    return info;
}

inline auto Semaphore(void *pNext = nullptr)
{
    VkSemaphoreCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = pNext;
    info.flags = 0;
    return info;
}

inline auto PipelineShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule, void *pNext = nullptr)
{
    VkPipelineShaderStageCreateInfo info {};
    info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext  = pNext;
    info.stage  = stage;         // shader stage
    info.module = shaderModule;  // module containing the code for this shader stage
    info.pName  = "main";        // the entry point of the shader
    return info;
}

inline auto VertexInputState(void *pNext = nullptr)
{
    VkPipelineVertexInputStateCreateInfo info {};
    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext                           = pNext;
    info.vertexBindingDescriptionCount   = 0;  // no vertex bindings or attributes
    info.vertexAttributeDescriptionCount = 0;  // no vertex bindings or attributes
    return info;
}

inline auto InputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, void *pNext = nullptr)
{
    VkPipelineInputAssemblyStateCreateInfo info {};
    info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext                  = pNext;
    info.topology               = topology;
    info.primitiveRestartEnable = VK_FALSE;  // we are not going to use primitive restart
    return info;
}

inline auto RasterizationState(
  VkCullModeFlagBits cullMode  = VK_CULL_MODE_BACK_BIT,
  VkFrontFace        frontFace = VK_FRONT_FACE_CLOCKWISE,
  void              *pNext     = nullptr)
{
    VkPipelineRasterizationStateCreateInfo info {};
    info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext                   = pNext;
    info.depthClampEnable        = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode             = VK_POLYGON_MODE_FILL;
    info.lineWidth               = 1.0f;
    // no backface cull
    info.cullMode                = cullMode;
    info.frontFace               = frontFace;
    // no depth bias
    info.depthBiasEnable         = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp          = 0.0f;
    info.depthBiasSlopeFactor    = 0.0f;

    return info;
}

inline auto MultisamplingState(VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, void *pNext = nullptr)
{
    VkPipelineMultisampleStateCreateInfo info {};
    info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext                 = pNext;
    info.sampleShadingEnable   = VK_FALSE;
    info.rasterizationSamples  = sampleCount;  // multisampling defaulted to no multisampling
    info.minSampleShading      = 1.0f;
    info.pSampleMask           = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable      = VK_FALSE;
    return info;
}

inline auto PipelineLayout(void *pNext = nullptr)
{
    VkPipelineLayoutCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = pNext;

    // @later
    info.flags                  = 0;
    info.setLayoutCount         = 0;
    info.pSetLayouts            = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges    = nullptr;

    return info;
}

}  // namespace CreateInfo

namespace AllocInfo
{

inline auto CommanddBuffer(
  VkCommandPool        pool,
  uint32_t             count = 1,
  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  void                *pNext = nullptr)
{
    VkCommandBufferAllocateInfo info {};
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext              = pNext;
    info.commandPool        = pool;
    info.commandBufferCount = count;
    info.level              = level;
    return info;
}

}  // namespace AllocInfo

namespace Create
{

inline VkShaderModule ShaderModule(VkDevice device, std::string const &name, VkShaderStageFlagBits stage)
{
    static umap<VkShaderStageFlagBits, std::string> sStageToExt {
        { VK_SHADER_STAGE_VERTEX_BIT, "vert" },
        { VK_SHADER_STAGE_FRAGMENT_BIT, "frag" },
        { VK_SHADER_STAGE_COMPUTE_BIT, "comp" },
    };

    // TODO / FIXME : On 'install' change shaders path to absolute from a selected resources-path
    // static auto const sShadersPath = std::string("./Assets/Shaders/");
    static auto const sShadersPath = std::string("./build/Assets/Shaders/");

    if (sStageToExt.count(stage) < 1)
    {
        BTM_ERR("Shaders support is limited to: .vert, .frag and .comp");
        return VK_NULL_HANDLE;
    }

    if (name.empty())
    {
        BTM_ERR("Shader name cannot be empty");
        return VK_NULL_HANDLE;
    }

    std::string const path = sShadersPath + name + "." + sStageToExt[stage] + ".spv";
    auto const        code = btm::fs::read(path);

    if (code.empty())
    {
        BTM_ERRF("Failed to open shader '{}'!", path);
        BTM_ASSERT(0);
        return {};
    }

    VkShaderModuleCreateInfo info {};
    info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext    = nullptr;
    info.codeSize = BTM_SIZEu32(code);
    info.pCode    = BTM_DATA(const uint32_t *, code);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &info, nullptr, &shaderModule) != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return shaderModule;
}

VkPipeline Pipeline(vk::PipelineBuilder pb, VkDevice device, VkRenderPass pass)
{
    // make viewport state from our stored viewport and scissor.
    // at the moment we won't support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext         = nullptr;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &pb.viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &pb.scissor;

    // setup dummy color blending. We aren't using transparent objects yet
    // the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext           = nullptr;
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.logicOp         = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &pb.colorBlendAttachment;

    // build the actual pipeline
    // we now use all of the info structs we have been writing into into this one to create the pipeline
    VkGraphicsPipelineCreateInfo info {};
    info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.pNext               = nullptr;
    info.stageCount          = pb.shaderStages.size();
    info.pStages             = pb.shaderStages.data();
    info.pVertexInputState   = &pb.vertexInputInfo;
    info.pInputAssemblyState = &pb.inputAssembly;
    info.pViewportState      = &viewportState;
    info.pRasterizationState = &pb.rasterizer;
    info.pMultisampleState   = &pb.multisampling;
    info.pColorBlendState    = &colorBlending;
    info.layout              = pb.pipelineLayout;
    info.renderPass          = pass;
    info.subpass             = 0;
    info.basePipelineHandle  = VK_NULL_HANDLE;

    // it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
    VkPipeline pipeline;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
    {
        BTM_ERR("Couldn't create pipeline");
        return VK_NULL_HANDLE;
    }

    return pipeline;
}

}  // namespace Create

}  // namespace btm::vk