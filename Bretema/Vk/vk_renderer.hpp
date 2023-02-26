#pragma once

#include "vk_base.hpp"
#include "vk_str.hpp"
#include "vk_types.hpp"

// ^^^ Include the <vk/dx/gl/mt/wg>-Renderer files before the BaseRenderer

#include "../btm_base.hpp"
#include "../btm_tools.hpp"
#include "../btm_renderer.hpp"

#include <vma/vk_mem_alloc.h>

namespace btm::vk
{

class Renderer : public btm::BaseRenderer
{
    static constexpr u64 sOneSec = 1000000000;

    struct MeshPushConstants
    {
        glm::vec4 data;
        glm::mat4 modelViewProj;
    };

public:
    Renderer(Ref<btm::Window> window);
    virtual void update() override { BTM_WARN("NOT IMPLEMENTED"); }
    virtual void draw() override;
    virtual void cleanup() override;

private:
    void initVulkan();
    void initSwapchain();
    void initCommands();
    void initDefaultRenderPass();
    void initFramebuffers();
    void initSyncStructures();
    void initPipelines();

    void loadMeshes();
    Mesh createMesh(btm::Vertices const &verts);

    inline VkExtent2D extent() { return VkExtent2D(mViewportSize.x, mViewportSize.y); }
    inline VkExtent3D extent3D() { return VkExtent3D(mViewportSize.x, mViewportSize.y, 1); }

    btm::ds::DeletionQueue mDestroyer {};  // add deletion funcs after create an vk-object, using:
                                           // mMainDelQueue.push_back([this]() {  });

    VkInstance               mInstance       = VK_NULL_HANDLE;  // Vulkan library handle
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;  // Vulkan debug output handle
    VkPhysicalDevice         mChosenGPU      = VK_NULL_HANDLE;  // GPU chosen as the default device
    VkDevice                 mDevice         = VK_NULL_HANDLE;  // Vulkan device for commands
    VmaAllocator             mAllocator      = VK_NULL_HANDLE;  // Memory Allocator - AMD lib
    VkSurfaceKHR             mSurface        = VK_NULL_HANDLE;  // Vulkan window surface (in a future could be an array)

    VkSwapchainKHR           mSwapchain            = VK_NULL_HANDLE;           // Vulkan swapchain
    VkFormat                 mSwapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;  // Image format expected by window
    std::vector<VkImage>     mSwapchainImages      = {};                       // List of images from the swapchain
    std::vector<VkImageView> mSwapchainImageViews  = {};                       // List of image-views from the swapchain

    vk::Queue mGraphicsQ = {};  // Queue/Family for Graphics
    vk::Queue mComputeQ  = {};  // Queue/Family for Compute
    vk::Queue mPresentQ  = {};  // Queue/Family for Present
    vk::Queue mTransferQ = {};  // Queue/Family for Transfer

    VkCommandPool   mCommandPool;        // Command pool for graphic-commands right now
    VkCommandBuffer mMainCommandBuffer;  // Main command buffer

    VkImageView    mDepthImageView         = VK_NULL_HANDLE;
    AllocatedImage mDepthImage             = {};
    static VkFormat constexpr sDepthFormat = VK_FORMAT_D32_SFLOAT;  // @todo: Check VK_FORMAT_D32_SFLOAT_S8_UINT  ??

    VkRenderPass               mDefaultRenderPass = VK_NULL_HANDLE;  // Basic renderpass config with (1) color and subpass
    std::vector<VkFramebuffer> mFramebuffers      = {};              // Bucket of FBOs, one per swapchain-image(view)

    // There is room to improve here, check:
    // * https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
    VkSemaphore mPresentSemaphore = VK_NULL_HANDLE;
    VkSemaphore mRenderSemaphore  = VK_NULL_HANDLE;
    VkFence     mRenderFence      = VK_NULL_HANDLE;

    std::vector<VkPipelineLayout> mPipelineLayouts = {};  // Bucket of pipeline-layouts
    std::vector<VkPipeline>       mPipelines       = {};  // Bucket of pipelines

    std::vector<Mesh> mMeshes = {};  // Bucket of mesehes
};

}  // namespace btm::vk
