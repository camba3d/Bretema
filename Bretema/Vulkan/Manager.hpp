#pragma once

#include "../Base.hpp"
#include "Base.hpp"
#include "ToStr.hpp"

namespace btm::vk
{

class Manager
{
public:
    void initialize(void *windowHandle, glm::vec2 const &viewportSize);

    void createSwapchain(glm::vec2 const &viewportSize);

    void createCommands();

    // NOTE: remember to call WindowManager::destroy(windowHandle) right after call this function
    void cleanup();

    inline bool isInitialized() { return mIsInitialized; }

private:
    VkInstance               mInstance;        // Vulkan library handle
    VkDebugUtilsMessengerEXT mDebugMessenger;  // Vulkan debug output handle
    VkPhysicalDevice         mChosenGPU;       // GPU chosen as the default device
    VkDevice                 mDevice;          // Vulkan device for commands
    VkSurfaceKHR             mSurface;         // Vulkan window surface (in a future could be an array)

    VkSwapchainKHR           mSwapchain;             // Vulkan swapchain
    VkFormat                 mSwapchainImageFormat;  // Image format expected by the windowing system
    std::vector<VkImage>     mSwapchainImages;       // List of images from the swapchain
    std::vector<VkImageView> mSwapchainImageViews;   // List of image-views from the swapchain

    VkQueue  mGraphicsQueue;        // queue we will submit to
    uint32_t mGraphicsQueueFamily;  // family of that queue

    VkCommandPool   mCommandPool;        // the command pool for our commands
    VkCommandBuffer mMainCommandBuffer;  // the buffer we will record into

    bool mIsInitialized = false;
};

}  // namespace btm::vk
