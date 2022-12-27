#include "vk_engine.hpp"

#include "../btm_window.hpp"
#include "vk_init.hpp"

#include <chrono>

namespace btm::vk
{

constexpr int BTM_VK_MAJOR_VERSION = 1;
constexpr int BTM_VK_MINOR_VERSION = 2;
#define BTM_VK_VER BTM_VK_MAJOR_VERSION, BTM_VK_MINOR_VERSION

void Engine::initialize(void *windowHandle, glm::vec2 const &viewportSize)
{
    // * https://github.com/charles-lunarg/vk-bootstrap/blob/master/docs/getting_started.md

    BTM_ASSERT_X(windowHandle, "Invalid window handle");
    BTM_ASSERT_X(viewportSize.x > 0 && viewportSize.y > 0, "Invalid viewport size");

    mWindowHandle = windowHandle;

    mExtent       = { (uint32_t)viewportSize.x, (uint32_t)viewportSize.y };
    mViewportSize = viewportSize;

    BTM_INFO("INIT...");
    initVulkan();
    BTM_INFO("initVulkan [DONE]");
    initSwapchain();
    BTM_INFO("initSwapchain [DONE]");
    initCommands();
    BTM_INFO("initCommands [DONE]");
    initDefaultrenderpass();
    BTM_INFO("initDefaultrenderpass [DONE]");
    initFramebuffers();
    BTM_INFO("initFramebuffers [DONE]");
    initSyncStructures();
    BTM_INFO("initSyncStructures [DONE]");

    mIsInitialized = true;
}

void Engine::initVulkan()
{
    // vkb : Create a instance with some setup
    auto vkbInstanceBuilder = vkb::InstanceBuilder {};
    auto vkbInstanceResult  = vkbInstanceBuilder.set_app_name("Bretema Default Engine")
                               .request_validation_layers(true)
                               .require_api_version(BTM_VK_VER, 0)
                               .use_default_debug_messenger()
                               .build();
    VKB_CHECK(vkbInstanceResult);
    auto vkbInstance = vkbInstanceResult.value();

    // Instance
    mInstance = vkbInstance.instance;

    // Debug messenger
    mDebugMessenger = vkbInstance.debug_messenger;

    // Surface : // @dani externalize this calls
    glfwCreateWindowSurface(mInstance, (GLFWwindow *)mWindowHandle, nullptr, &mSurface);

    // vkb : Select a GPU based on some criteria
    auto vkbGpuSelector = vkb::PhysicalDeviceSelector { vkbInstance };
    auto vkbGpuResult   = vkbGpuSelector.set_minimum_version(BTM_VK_VER).set_surface(mSurface).select();
    VKB_CHECK(vkbGpuResult);
    auto vkbGpu = vkbGpuResult.value();

    // Physical Device  (GPU)
    mChosenGPU = vkbGpu.physical_device;

    // vkb : Create the final Vulkan device
    auto vkbDeviceBuilder = vkb::DeviceBuilder { vkbGpu };
    auto vkbDeviceResult  = vkbDeviceBuilder.build();
    VKB_CHECK(vkbDeviceResult);
    auto vkbDevice = vkbDeviceResult.value();

    // Device
    mDevice = vkbDevice.device;

    // Queues
    mGraphicsQ = { vkbDevice, vkb::QueueType::graphics };
    mComputeQ  = { vkbDevice, vkb::QueueType::compute };
    mPresentQ  = { vkbDevice, vkb::QueueType::present };
    mTransferQ = { vkbDevice, vkb::QueueType::transfer };
}

void Engine::initSwapchain()
{
    BTM_ASSERT_X(mViewportSize.x > 0 && mViewportSize.y > 0, "Invalid viewport size");

    // vkb : Create swapchain
    auto vkbSwapchainBuilder = vkb::SwapchainBuilder { mChosenGPU, mDevice, mSurface };
    auto vkbSwapchainResult  = vkbSwapchainBuilder.use_default_format_selection()
                                .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)  // fifo = vsync present mode
                                .set_desired_extent(mViewportSize.x, mViewportSize.y)
                                .use_default_image_usage_flags()
                                .set_desired_min_image_count(sInFlight)
                                .build();
    VKB_CHECK(vkbSwapchainResult);
    auto vkbSwapchain = vkbSwapchainResult.value();
    mSwapchain        = vkbSwapchain.swapchain;

    mExtent       = vkbSwapchain.extent;
    mViewportSize = { mExtent.width, mExtent.height };

    auto vkbSwapchainImagesResult = vkbSwapchain.get_images();
    VKB_CHECK(vkbSwapchainImagesResult);
    mSwapchainImages = vkbSwapchainImagesResult.value();

    auto vkbSwapchainImageViewsResult = vkbSwapchain.get_image_views();
    VKB_CHECK(vkbSwapchainImageViewsResult);
    mSwapchainImageViews = vkbSwapchainImageViewsResult.value();

    mSwapchainImageFormat = vkbSwapchain.image_format;
}

void Engine::initCommands()
{
    // Create a command pool for commands submitted to the graphics queue
    auto const cmdPoolFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    auto const cmdPoolCI    = init::cmdPoolCreateInfo(mGraphicsQ.family, cmdPoolFlags);
    VK_CHECK(vkCreateCommandPool(mDevice, &cmdPoolCI, nullptr, &mCommandPool));

    // Allocate the default command buffer that we will use for rendering
    auto const cmdAllocInfo = init::cmdBufferAllocInfo(mCommandPool);
    VK_CHECK(vkAllocateCommandBuffers(mDevice, &cmdAllocInfo, &mMainCommandBuffer));
}

void Engine::initDefaultrenderpass()
{
    VkAttachmentDescription color0 = {};
    color0.format                  = mSwapchainImageFormat;             // Copy swapchain format
    color0.samples                 = VK_SAMPLE_COUNT_1_BIT;             // 1 sample, no MSAA right now
    color0.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;       // Clear on load
    color0.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;      // Keep stored on renderpass end
    color0.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // No stencil right now
    color0.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // No stencil right now
    color0.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;         // Let it as undefined on init
    color0.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // Ready to display on renderpass end

    VkAttachmentReference refColor0 = {};
    refColor0.attachment            = 0;  // Attachment idx in the renderpass
    refColor0.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;  // Create the renderpass for graphics
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &refColor0;

    VkRenderPassCreateInfo renderpassCI = {};
    renderpassCI.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // Connect the attachment to the info
    renderpassCI.attachmentCount        = 1;
    renderpassCI.pAttachments           = &color0;
    // Connect the subpass to the info
    renderpassCI.subpassCount           = 1;
    renderpassCI.pSubpasses             = &subpass;

    VK_CHECK(vkCreateRenderPass(mDevice, &renderpassCI, nullptr, &mDefaultRenderPass));
}

void Engine::initFramebuffers()
{
    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCI = {};
    framebufferCI.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCI.pNext                   = nullptr;
    framebufferCI.renderPass              = mDefaultRenderPass;
    framebufferCI.attachmentCount         = 1;
    framebufferCI.width                   = mExtent.width;
    framebufferCI.height                  = mExtent.height;
    framebufferCI.layers                  = 1;

    // Grab how many images we have in the swapchain
    mFramebuffers = decltype(mFramebuffers)(mSwapchainImages.size());

    // Create framebuffers for each of the swapchain image views
    for (size_t i = 0; i < mSwapchainImages.size(); i++)
    {
        framebufferCI.pAttachments = &mSwapchainImageViews[i];
        VK_CHECK(vkCreateFramebuffer(mDevice, &framebufferCI, nullptr, &mFramebuffers[i]));
    }
}

void Engine::initSyncStructures()
{
    auto const fenceCI = init::fenceCreateInfo();
    VK_CHECK(vkCreateFence(mDevice, &fenceCI, nullptr, &mRenderFence));

    auto const semaphoreCI = init::semaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCI, nullptr, &mPresentSemaphore));
    VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCI, nullptr, &mRenderSemaphore));
}

void Engine::initPipelines() {}

void Engine::draw()
{
    uint64_t _1s = 1000000000;

    auto cmd = mMainCommandBuffer;

    // Wait for GPU (1 second timeout)
    VK_CHECK(vkWaitForFences(mDevice, 1, &mRenderFence, true, _1s));
    VK_CHECK(vkResetFences(mDevice, 1, &mRenderFence));

    // Request image from the swapchain (1 second timeout)
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapchain, _1s, mPresentSemaphore, nullptr, &swapchainImageIndex));

    // Reset command buffer
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // Begin the command buffer recording.
    // We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo cmdBI = {};
    cmdBI.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBI.pNext                    = nullptr;
    cmdBI.pInheritanceInfo         = nullptr;
    cmdBI.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBI));

    // Make a clear-color from frame number.
    // This will flash with a 120*pi frame period.
    VkClearValue clear;
    clear.color = { { 0.0f, 0.0f, float(abs(sin(mFrameNumber / 120.f))), 1.0f } };

    // Start the main renderpass.
    // We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo renderpassBI = {};
    renderpassBI.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBI.pNext                 = nullptr;
    renderpassBI.renderPass            = mDefaultRenderPass;
    renderpassBI.renderArea.offset.x   = 0;
    renderpassBI.renderArea.offset.y   = 0;
    renderpassBI.renderArea.extent     = mExtent;
    renderpassBI.framebuffer           = mFramebuffers[swapchainImageIndex];
    renderpassBI.clearValueCount       = 1;
    renderpassBI.pClearValues          = &clear;
    vkCmdBeginRenderPass(cmd, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);

    // Finalize the render pass
    vkCmdEndRenderPass(cmd);

    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    // Prepare the submission to the queue.
    // We want to wait on the mPresentSemaphore, as that semaphore is signaled when the swapchain is ready
    // we will signal the mRenderSemaphore, to signal that rendering has finished
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo         = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = nullptr;
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &mPresentSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &mRenderSemaphore;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &cmd;

    VK_CHECK(vkQueueSubmit(mGraphicsQ.queue, 1, &submitInfo, mRenderFence));  // Submit and execute
    // --> mRenderFence will now block until the graphic commands finish execution

    // This will put the image we just rendered into the visible window.
    // We want to wait on the mRenderSemaphore for that, as it's necessary that drawing commands have finished
    //  before the image is displayed to the user
    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.pSwapchains        = &mSwapchain;
    presentInfo.swapchainCount     = 1;
    presentInfo.pWaitSemaphores    = &mRenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices      = &swapchainImageIndex;
    VK_CHECK(vkQueuePresentKHR(mGraphicsQ.queue, &presentInfo));

    // Increase the number of frames drawn
    mFrameNumber++;
}

void Engine::cleanup()
{
    if (!mIsInitialized)
        return;

    // Sync objects
    vkDestroyFence(mDevice, mRenderFence, nullptr);
    vkDestroySemaphore(mDevice, mRenderSemaphore, nullptr);
    vkDestroySemaphore(mDevice, mPresentSemaphore, nullptr);

    // Commands
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

    // Default RenderPass
    vkDestroyRenderPass(mDevice, mDefaultRenderPass, nullptr);

    // Swapchain Resources : framebuffers
    for (int32_t i = mFramebuffers.size() - 1; i >= 0; --i)
        if (mFramebuffers[i])
            vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr);

    // Swapchain Resources : imageviews
    for (int32_t i = mSwapchainImageViews.size() - 1; i >= 0; --i)
        if (mSwapchainImageViews[i])
            vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);

    // Swapchain
    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

    // Device
    vkDestroyDevice(mDevice, nullptr);

    // Surface (in a future could be more than one surface)
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

    // Instance
    vkb::destroy_debug_utils_messenger(mInstance, mDebugMessenger);
    vkDestroyInstance(mInstance, nullptr);
}

}  // namespace btm::vk
