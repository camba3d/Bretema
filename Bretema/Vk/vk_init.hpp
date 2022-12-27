#pragma once

#include "../btm_base.hpp"
#include "../btm_tools.hpp"
#include "vk_base.hpp"

#include <optional>
#include <vector>

// From : https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanInitializers.hpp

namespace btm::vk::init
{

inline auto cmdPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0, void *pNext = nullptr)
{
    VkCommandPoolCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext                   = pNext;
    info.queueFamilyIndex        = queueFamilyIndex;
    info.flags                   = flags;

    return info;
}

inline auto cmdBufferAllocInfo(
  VkCommandPool        pool,
  uint32_t             count = 1,
  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
  void                *pNext = nullptr)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext                       = pNext;
    info.commandPool                 = pool;
    info.commandBufferCount          = count;
    info.level                       = level;

    return info;
}

inline auto fenceCreateInfo(VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT, void *pNext = nullptr)
{
    VkFenceCreateInfo info = {};
    info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext             = pNext;
    info.flags             = flags;

    return info;
}

inline auto semaphoreCreateInfo(void *pNext = nullptr)
{
    VkSemaphoreCreateInfo info = {};
    info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext                 = pNext;
    info.flags                 = 0;

    return info;
}

inline std::optional<VkShaderModule>
  createShaderModule(VkDevice device, std::string const &name, VkShaderStageFlagBits stage)
{
    static umap<VkShaderStageFlagBits, std::string> sStageToExt {
        { VK_SHADER_STAGE_VERTEX_BIT, "vert" },
        { VK_SHADER_STAGE_FRAGMENT_BIT, "frag" },
        { VK_SHADER_STAGE_COMPUTE_BIT, "comp" },
    };
    static auto const sShadersPath = std::string("./assets/shaders/");  // get this path from #define

    if (!name.empty() and sStageToExt.count(stage) > 0)
    {
        BTM_ASSERT(0);
        return {};
    }

    std::string const path = sShadersPath + name + "." + sStageToExt[stage] + ".spv";
    std::string const code = btm::fs::read(path);

    if (code.empty())
    {
        BTM_ASSERT(0);
        BTM_ERRF("Failed to open shader '{}'!", path);
        return {};
    }

    VkShaderModuleCreateInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext                    = nullptr;
    info.codeSize                 = BTM_SIZEOFu32(code);
    info.pCode                    = BTM_DATA(const uint32_t *, code);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &info, nullptr, &shaderModule) != VK_SUCCESS)
        return {};

    return shaderModule;
}

}  // namespace btm::vk::init