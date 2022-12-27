#pragma once

#include "../btm_base.hpp"
#include "vk_base.hpp"
#include "vk_str.hpp"

namespace btm::vk
{

struct VulkanQueue
{
    VulkanQueue() = default;

    VulkanQueue(vkb::Device vkbDevice, vkb::QueueType queueType)
    {
        auto q = vkbDevice.get_queue(queueType);
        auto f = vkbDevice.get_queue_index(queueType);

        if (valid = q.has_value() && f.has_value(); valid)
        {
            queue  = q.value();
            family = f.value();
        }

        BTM_ASSERT(valid);
    }

    VkQueue  queue  = {};
    uint32_t family = {};
    bool     valid  = false;
};

struct VulkanPipelineBuilder
{
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    VkPipelineVertexInputStateCreateInfo         _vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo       _inputAssembly;
    VkViewport                                   _viewport;
    VkRect2D                                     _scissor;
    VkPipelineRasterizationStateCreateInfo       _rasterizer;
    VkPipelineColorBlendAttachmentState          _colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo         _multisampling;
    VkPipelineLayout                             _pipelineLayout;

    VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};

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

    VulkanQueue mGraphicsQ = {};  // Queue/Family for Graphics
    VulkanQueue mComputeQ  = {};  // Queue/Family for Compute
    VulkanQueue mPresentQ  = {};  // Queue/Family for Present
    VulkanQueue mTransferQ = {};  // Queue/Family for Transfer

    VkCommandPool   mCommandPool;        // Command pool for graphic-commands right now
    VkCommandBuffer mMainCommandBuffer;  // Main command buffer

    VkRenderPass mDefaultRenderPass          = VK_NULL_HANDLE;  // Basic renderpass config with (1) color and subpass
    std::vector<VkFramebuffer> mFramebuffers = {};              // List of FBOs, one per swapchain-image(view)

    VkSemaphore mPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore mRenderSemaphore  = VK_NULL_HANDLE;
    VkFence     mRenderFence      = VK_NULL_HANDLE;

    bool mIsInitialized = false;
};

}  // namespace btm::vk
