#pragma once

#include "Base.hpp"

namespace btm::vk
{

class Manager
{
    void initialize(std::vector<void *> windowHandles);
    void createSwapchain();

    inline bool isInitialized() { return mIsInitialized; }

private:
    VkInstance               mInstance;       // Vulkan library handle
    VkDebugUtilsMessengerEXT mDebugMessenger; // Vulkan debug output handle
    VkPhysicalDevice         mChosenGPU;      // GPU chosen as the default device
    VkDevice                 mDevice;         // Vulkan device for commands
    VkSurfaceKHR             mSurface;        // Vulkan window surface

    VkSwapchainKHR           mSwapchain;            // Vulkan swapchain
    VkFormat                 mSwapchainImageFormat; // Image format expected by the windowing system
    std::vector<VkImage>     mSwapchainImages;      // List of images from the swapchain
    std::vector<VkImageView> mSwapchainImageViews;  // List of image-views from the swapchain

    bool mIsInitialized = false;
};

} // namespace btm::vk
