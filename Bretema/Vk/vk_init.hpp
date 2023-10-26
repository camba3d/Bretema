#pragma once

#include "../btm_base.hpp"
#include "../btm_utils.hpp"

#include "vk_base.hpp"
#include "vk_types.hpp"

#include <optional>
#include <vector>

namespace btm::vk
{

namespace CreateInfo
{

inline auto CommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags = 0, void *pNext = nullptr)
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

inline auto VertexInputState(VertexInputDescription const &desc = {}, void *pNext = nullptr)
{
    VkPipelineVertexInputStateCreateInfo info {};

    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext                           = pNext;
    info.vertexBindingDescriptionCount   = 0;
    info.vertexAttributeDescriptionCount = 0;
    if (desc.bindings.size() >= 1 && desc.bindings.data())
    {
        info.vertexBindingDescriptionCount = (u32)desc.bindings.size();
        info.pVertexBindingDescriptions    = desc.bindings.data();
    }
    if (desc.attributes.size() >= 1 && desc.attributes.data())
    {
        info.vertexAttributeDescriptionCount = (u32)desc.attributes.size();
        info.pVertexAttributeDescriptions    = desc.attributes.data();
    }

    return info;
}

inline auto InputAssembly(void *pNext = nullptr)
{
    VkPipelineInputAssemblyStateCreateInfo info {};

    info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext                  = pNext;
    info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.primitiveRestartEnable = VK_FALSE;  // we are not going to use primitive restart

    return info;
}

inline auto RasterizationState(Cull cullMode = Cull::CCW, void *pNext = nullptr)
{
    VkPipelineRasterizationStateCreateInfo info {};

    info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext                   = pNext;
    info.depthClampEnable        = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode             = VK_POLYGON_MODE_FILL;
    info.lineWidth               = 1.0f;

    // culling
    info.cullMode  = static_cast<VkCullModeFlagBits>(cullMode);
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // no depth bias
    info.depthBiasEnable         = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp          = 0.0f;
    info.depthBiasSlopeFactor    = 0.0f;

    return info;
}

inline auto MultisamplingState(Samples samples = Samples::_1, void *pNext = nullptr)
{
    VkPipelineMultisampleStateCreateInfo info {};

    info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext                 = pNext;
    info.sampleShadingEnable   = VK_FALSE;
    info.rasterizationSamples  = static_cast<VkSampleCountFlagBits>(BTM_BIT((i32)samples));
    info.minSampleShading      = 1.0f;
    info.pSampleMask           = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable      = VK_FALSE;

    return info;
}

inline auto PipelineLayout(void *pNext = nullptr)
{
    VkPipelineLayoutCreateInfo info {};

    info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext                  = pNext;
    // @later
    info.flags                  = 0;
    info.setLayoutCount         = 0;
    info.pSetLayouts            = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges    = nullptr;

    return info;
}

inline auto Image(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, Samples samples = Samples::_1, void *pNext = nullptr)
{
    VkImageCreateInfo info {};

    info.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext       = pNext;
    info.imageType   = VK_IMAGE_TYPE_2D;  //@todo: extend to cube maps and 3d textures
    info.format      = format;
    info.extent      = extent;
    info.mipLevels   = 1;  //@todo
    info.arrayLayers = 1;  //@todo: extend to cube maps and 3d textures
    info.samples     = static_cast<VkSampleCountFlagBits>(BTM_BIT((i32)samples));
    info.tiling      = VK_IMAGE_TILING_OPTIMAL;
    info.usage       = usageFlags;

    return info;
}

//@todo: at some point, may have sense to merge this two functions in btm::vk::Image class, that tracks
// the image type for the view creation, and reduce verbosity while allowing modify any parameter of the structs...
inline auto ImageView(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, void *pNext = nullptr)
{
    VkImageViewCreateInfo info {};

    info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext                           = pNext;
    info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;  //@todo: extend to cube maps and 3d textures
    info.image                           = image;
    info.format                          = format;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;
    info.subresourceRange.aspectMask     = aspectFlags;

    return info;
}

inline auto DepthStencil(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp, void *pNext = nullptr)
{
    VkPipelineDepthStencilStateCreateInfo info {};

    info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext                 = pNext;
    info.depthTestEnable       = bDepthTest ? VK_TRUE : VK_FALSE;
    info.depthWriteEnable      = bDepthWrite ? VK_TRUE : VK_FALSE;
    info.depthCompareOp        = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
    info.depthBoundsTestEnable = VK_FALSE;  //@optional
    info.minDepthBounds        = 0.0f;      //@optional
    info.maxDepthBounds        = 1.0f;      //@optional
    info.stencilTestEnable     = VK_FALSE;  //@todo

    return info;
}

}  // namespace CreateInfo

namespace AllocInfo
{

inline auto
  CommandBuffer(VkCommandPool pool, u32 count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, void *pNext = nullptr)
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
// BTM_INFOF("AAAAAAAAAAAAAAAAAA {}", runtime::exepath());
#ifdef _MSC_VER
    static auto const sShadersPath = runtime::exepath() + "/../Assets/Shaders/";
#else
    static auto const sShadersPath = runtime::exepath() + "/Assets/Shaders/";
#endif

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
    info.codeSize = BMVK_COUNT(code);
    info.pCode    = BMVK_DATAC(u32, code);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &info, nullptr, &shaderModule) != VK_SUCCESS) return VK_NULL_HANDLE;

    return shaderModule;
}

inline VkPipeline Pipeline(vk::PipelineBuilder pb, VkDevice device, VkRenderPass pass, std::vector<VkDynamicState> dynamicStates)
{
    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (u32)dynamicStates.size();
    dynamicState.pDynamicStates    = dynamicStates.data();

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
    info.stageCount          = (u32)pb.shaderStages.size();
    info.pStages             = pb.shaderStages.data();
    info.pVertexInputState   = &pb.vertexInputInfo;
    info.pInputAssemblyState = &pb.inputAssembly;
    info.pViewportState      = &viewportState;
    info.pRasterizationState = &pb.rasterizer;
    info.pMultisampleState   = &pb.multisampling;
    info.pColorBlendState    = &colorBlending;
    info.pDynamicState       = &dynamicState;
    info.layout              = pb.pipelineLayout;
    info.renderPass          = pass;
    info.subpass             = 0;
    info.pDepthStencilState  = &pb.depthStencil;
    info.basePipelineHandle  = VK_NULL_HANDLE;

    // it's easy to error out on create graphics pipeline, so we handle it a bit better than the common BMVK_CHECK case
    VkPipeline pipeline;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
    {
        BTM_ERR("Couldn't create pipeline");
        return VK_NULL_HANDLE;
    }

    return pipeline;
}

struct DescSetLayoutBinding_t
{
    VkDescriptorType   type    = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    VkShaderStageFlags stages  = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    uint32_t           binding = 0;  // Should we use UINT32_MAX instead?
};
inline auto DescSetLayout(VkDevice device, std::vector<DescSetLayoutBinding_t> inLayoutBindings, VkDescriptorSetLayoutCreateFlags flags = 0)
{
    VkDescriptorSetLayout descSetLayout;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (auto const &in : inLayoutBindings)
    {
        VkDescriptorSetLayoutBinding out = {};
        out.binding                      = in.binding;
        out.descriptorCount              = 1;
        out.descriptorType               = in.type;
        out.pImmutableSamplers           = nullptr;
        out.stageFlags                   = in.stages;
        layoutBindings.push_back(out);
    }

    VkDescriptorSetLayoutCreateInfo CI = {};
    CI.bindingCount                    = (u32)layoutBindings.size();
    CI.flags                           = flags;
    CI.pNext                           = nullptr;
    CI.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    CI.pBindings                       = layoutBindings.data();

    BMVK_CHECK(vkCreateDescriptorSetLayout(device, &CI, nullptr, &descSetLayout));

    return descSetLayout;
}

}  // namespace Create

namespace Update
{

struct WriteDescSet_t
{
    VkDescriptorType type    = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    VkDescriptorSet  dstSet  = VK_NULL_HANDLE;
    uint32_t         binding = 0;  // Should we use UINT32_MAX instead?
    VkBuffer         buffer  = VK_NULL_HANDLE;
    VkDeviceSize     offset  = 0;
    VkDeviceSize     range   = 0;
};

inline auto DescSets(VkDevice dev, std::vector<WriteDescSet_t> inWriteDescSets)
{
    std::vector<VkDescriptorBufferInfo> buffInfos;
    for (auto const &in : inWriteDescSets)
    {
        buffInfos.emplace_back(in.buffer, in.offset, in.range);
    }

    int i = 0;

    std::vector<VkWriteDescriptorSet> writeDescSets = {};
    for (auto const &in : inWriteDescSets)
    {
        VkWriteDescriptorSet out = {};
        out.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        out.pNext                = nullptr;
        out.dstBinding           = in.binding;
        out.dstSet               = in.dstSet;
        out.descriptorCount      = 1;
        out.descriptorType       = in.type;
        out.pBufferInfo          = &buffInfos[i++];
        writeDescSets.push_back(out);
    }

    auto const &bi = writeDescSets[0].pBufferInfo[0];
    BTM_INFOF("FFFFF =>> {} {} {}", BTM_STR_PTR(bi.buffer), bi.offset, bi.range);

    vkUpdateDescriptorSets(dev, (u32)writeDescSets.size(), writeDescSets.data(), 0, nullptr);
}

}  // namespace Update

}  // namespace btm::vk