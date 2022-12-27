#pragma once

#include "../btm_base.hpp"
#include "vk_base.hpp"
#include "vk_str.hpp"
#include "vk_types.hpp"

namespace btm::vk
{

class Engine
{
public:
    inline static constexpr int32_t sInFlight = 3;

    void initialize(void *windowHandle, glm::vec2 const &viewportSize);

    void draw();

    // NOTE: remember to call WindowManager::destroy(windowHandle) right after call this function
    void cleanup();

    inline bool isInitialized() { return mIsInitialized; }

private:
    void initVulkan();
    void initSwapchain();
    void initCommands();
    void initDefaultrenderpass();
    void initFramebuffers();
    void initSyncStructures();
    void initPipelines();

    void *mWindowHandle = nullptr;

    glm::vec2  mViewportSize = { 1280, 720 };
    VkExtent2D mExtent       = { 1280, 720 };

    int32_t mFrameNumber = 0;

    VkInstance               mInstance       = VK_NULL_HANDLE;  // Vulkan library handle
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;  // Vulkan debug output handle
    VkPhysicalDevice         mChosenGPU      = VK_NULL_HANDLE;  // GPU chosen as the default device
    VkDevice                 mDevice         = VK_NULL_HANDLE;  // Vulkan device for commands
    VkSurfaceKHR             mSurface        = VK_NULL_HANDLE;  // Vulkan window surface (in a future could be an array)

    VkSwapchainKHR           mSwapchain            = VK_NULL_HANDLE;           // Vulkan swapchain
    VkFormat                 mSwapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;  // Image format expected by window
    std::vector<VkImage>     mSwapchainImages      = {};                       // List of images from the swapchain
    std::vector<VkImageView> mSwapchainImageViews  = {};                       // List of image-views from the swapchain

    vk::types::Queue mGraphicsQ = {};  // Queue/Family for Graphics
    vk::types::Queue mComputeQ  = {};  // Queue/Family for Compute
    vk::types::Queue mPresentQ  = {};  // Queue/Family for Present
    vk::types::Queue mTransferQ = {};  // Queue/Family for Transfer

    VkCommandPool   mCommandPool;        // Command pool for graphic-commands right now
    VkCommandBuffer mMainCommandBuffer;  // Main command buffer

    VkRenderPass mDefaultRenderPass          = VK_NULL_HANDLE;  // Basic renderpass config with (1) color and subpass
    std::vector<VkFramebuffer> mFramebuffers = {};              // List of FBOs, one per swapchain-image(view)

    VkSemaphore mPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore mRenderSemaphore  = VK_NULL_HANDLE;
    VkFence     mRenderFence      = VK_NULL_HANDLE;

    // shaders

    // pipelines

    bool mIsInitialized = false;
};

}  // namespace btm::vk
