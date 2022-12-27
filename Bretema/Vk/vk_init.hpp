#pragma once

#include "vk_base.hpp"

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

}  // namespace btm::vk::init