#pragma once

#include "Base.hpp"

#include <vector>

// From : https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanInitializers.hpp

namespace btm::vk::init
{

inline VkCommandPoolCreateInfo cmdPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0)
{
    VkCommandPoolCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext                   = nullptr;

    info.queueFamilyIndex = queueFamilyIndex;
    info.flags            = flags;

    return info;
}

VkCommandBufferAllocateInfo cmdBufferAllocInfo(
  VkCommandPool        pool,
  uint32_t             count = 1,
  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext                       = nullptr;

    info.commandPool        = pool;
    info.commandBufferCount = count;
    info.level              = level;

    return info;
}

}  // namespace btm::vk::init