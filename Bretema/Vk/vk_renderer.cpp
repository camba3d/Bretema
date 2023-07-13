#include "vk_renderer.hpp"
#include "vk_init.hpp"

#include <chrono>

namespace btm::vk
{

constexpr i32 BTM_VK_MAJOR_VERSION = 1;
constexpr i32 BTM_VK_MINOR_VERSION = 2;
#define BTM_VK_VER BTM_VK_MAJOR_VERSION, BTM_VK_MINOR_VERSION

#define TO_DESTROY(code__) mDeletionQueue.add([=, this]() { code__; })

//-----------------------------------------------------------------------------

Renderer::Renderer(sPtr<btm::Window> window) : btm::BaseRenderer(window)
{
    // * https://github.com/charles-lunarg/vk-bootstrap/blob/master/docs/getting_started.md

    initVulkan();
    initSwapchain();

    initDefaultRenderPass();
    initFramebuffers();

    initCommands();
    initSyncStructures();

    initMaterials();
    initMeshes();
    initTestScene();

    // init_descriptors();
    // init_pipelines();
    // load_images();
    // init_scene();

    markAsInit();
}

void Renderer::draw(Camera const &cam)
{
    // Wait for GPU (1 second timeout)
    VK_CHECK(vkWaitForFences(mDevice, 1, &frame().renderFence, true, sOneSec));
    VK_CHECK(vkResetFences(mDevice, 1, &frame().renderFence));

    // Request image from the swapchain (1 second timeout)
    u32 swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapchain, sOneSec, frame().presentSemaphore, nullptr, &swapchainImageIndex));

    // Reset command buffer
    VK_CHECK(vkResetCommandBuffer(frame().graphics.cmd, 0));

    // Begin the command buffer recording.
    // We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo cbBeginInfo {};
    cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(frame().graphics.cmd, &cbBeginInfo));

    // Make a clear-color from frame number.
    // This will flash with a 120*pi frame period.
    VkClearValue clearColor, clearDepth;
    clearColor.color              = { { 0.0f, 0.0f, float(abs(sin(mFrameNumber / 120.f))), 1.0f } };
    clearDepth.depthStencil.depth = 1.f;
    auto const clears             = std::array { clearColor, clearDepth };

    // Start the main renderpass.
    // We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo renderpassBI = {};
    renderpassBI.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBI.pNext                 = nullptr;
    renderpassBI.renderPass            = mDefaultRenderPass;
    renderpassBI.renderArea.offset.x   = 0;
    renderpassBI.renderArea.offset.y   = 0;
    renderpassBI.renderArea.extent     = extent2D();
    renderpassBI.framebuffer           = mFramebuffers[swapchainImageIndex];
    renderpassBI.clearValueCount       = (u32)clears.size();
    renderpassBI.pClearValues          = clears.data();

    vkCmdBeginRenderPass(frame().graphics.cmd, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);

    //===========

    drawScene("test", cam);

    //===========

    vkCmdEndRenderPass(frame().graphics.cmd);
    VK_CHECK(vkEndCommandBuffer(frame().graphics.cmd));

    // Prepare the submission to the queue.
    // We want to wait on the mPresentSemaphore, as that semaphore is signaled when the swapchain is ready
    // we will signal the mRenderSemaphore, to signal that rendering has finished
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo         = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = nullptr;
    submitInfo.pWaitDstStageMask    = &waitStage;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &frame().presentSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &frame().renderSemaphore;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &frame().graphics.cmd;

    VK_CHECK(vkQueueSubmit(mGraphics.queue, 1, &submitInfo, frame().renderFence));  // Submit and execute
    // --> mRenderFence will now block until the graphic commands finish execution

    // This will put the image we just rendered into the visible window.
    // We want to wait on the mRenderSemaphore for that, as it's necessary that drawing commands have finished
    //  before the image is displayed to the user
    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.pSwapchains        = &mSwapchain;
    presentInfo.swapchainCount     = 1;
    presentInfo.pWaitSemaphores    = &frame().renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices      = &swapchainImageIndex;
    VK_CHECK(vkQueuePresentKHR(mGraphics.queue, &presentInfo));

    // Increase the number of frames drawn
    mFrameNumber++;
}

void Renderer::cleanup()
{
    if (!isInitialized())
    {
        return;
    }

    for (u64 i = 0; i < sFlightFrames; i++)
    {
        vkWaitForFences(mDevice, 1, &mFrames[i].renderFence, true, sOneSec * 4);
    }

    mDeletionQueue.flush();

    vmaDestroyAllocator(mAllocator);

    vkDestroyDevice(mDevice, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkb::destroy_debug_utils_messenger(mInstance, mDebugMessenger);
    vkDestroyInstance(mInstance, nullptr);
}

//-----------------------------------------------------------------------------

void Renderer::initVulkan()
{
    // vkb : Create a instance with some setup
    auto vkbInstanceBuilder = vkb::InstanceBuilder {};

    for (auto &&ext : btm::Window::extensions())
    {
        vkbInstanceBuilder.enable_extension(ext);
    }

    auto vkbInstanceResult = vkbInstanceBuilder.set_app_name("Bretema Default Engine")
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

    // Surface : // @dani externalize this call ??
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

    // Initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = mChosenGPU;
    allocatorInfo.device                 = mDevice;
    allocatorInfo.instance               = mInstance;
    vmaCreateAllocator(&allocatorInfo, &mAllocator);

    // Queues
    mGraphics = vk::Queue { vkbDevice, vkb::QueueType::graphics };
    mPresent  = vk::Queue { vkbDevice, vkb::QueueType::present };
    mCompute  = vk::Queue { vkbDevice, vkb::QueueType::compute };
    mTransfer = vk::Queue { vkbDevice, vkb::QueueType::transfer };
    BTM_INFOF("G:{} | P:{} | C:{} | T:{}", mGraphics, mPresent, mCompute, mTransfer);
}

void Renderer::initSwapchain()
{
    BTM_ASSERT_X(mViewportSize.x > 0 && mViewportSize.y > 0, "Invalid viewport size");

    // === SWAP CHAIN ===

    // vkb : Create swapchain
    auto vkbSwapchainBuilder = vkb::SwapchainBuilder { mChosenGPU, mDevice, mSurface };
    auto vkbSwapchainResult  = vkbSwapchainBuilder.use_default_format_selection()
                                .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)  // fifo = vsync present mode
                                .set_desired_extent((u32)mViewportSize.x, (u32)mViewportSize.y)
                                .use_default_image_usage_flags()
                                .set_desired_min_image_count(sInFlight)
                                .build();

    VKB_CHECK(vkbSwapchainResult);
    auto vkbSwapchain = vkbSwapchainResult.value();
    mSwapchain        = vkbSwapchain.swapchain;

    // Swapchain images
    auto vkbSwapchainImagesResult = vkbSwapchain.get_images();
    VKB_CHECK(vkbSwapchainImagesResult);
    mSwapchainImages = vkbSwapchainImagesResult.value();

    // Swapchain image-views
    auto vkbSwapchainImageViewsResult = vkbSwapchain.get_image_views();
    VKB_CHECK(vkbSwapchainImageViewsResult);
    mSwapchainImageViews = vkbSwapchainImageViewsResult.value();

    // Swapchain image-format and viewport
    mSwapchainImageFormat = vkbSwapchain.image_format;
    mViewportSize         = { vkbSwapchain.extent.width, vkbSwapchain.extent.height };

    // Swapchain deletion-queue
    mDeletionQueue.add([this]() { vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr); });

    // === DEPTH BUFFER ===

    static auto const depthBit = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    auto const        imgInfo  = vk::CreateInfo::Image(sDepthFormat, depthBit, extent3D());

    // for the depth image, we want to allocate it from GPU local memory
    VmaAllocationCreateInfo imgAllocInfo = {};
    imgAllocInfo.usage                   = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    imgAllocInfo.requiredFlags           = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // allocate and create the image
    vmaCreateImage(mAllocator, &imgInfo, &imgAllocInfo, &mDepthImage.image, &mDepthImage.allocation, nullptr);
    TO_DESTROY(vmaDestroyImage(mAllocator, mDepthImage.image, mDepthImage.allocation));

    // build an image-view for the depth image to use for rendering
    auto const viewInfo = vk::CreateInfo::ImageView(sDepthFormat, mDepthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(mDevice, &viewInfo, nullptr, &mDepthImageView));
    TO_DESTROY(vkDestroyImageView(mDevice, mDepthImageView, nullptr));
}

void Renderer::initCommands()
{
    auto const initCommandsByFamily = [this](vk::QueueCmd &qc, vk::Queue *q)
    {
        auto const flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        qc.queue = sPtr<vk::Queue>(q);

        auto const cpInfo = vk::CreateInfo::CommandPool(q->family, flags);
        VK_CHECK(vkCreateCommandPool(mDevice, &cpInfo, nullptr, &qc.pool));
        TO_DESTROY(vkDestroyCommandPool(mDevice, qc.pool, nullptr));

        auto const cbAllocInfo = vk::AllocInfo::CommandBuffer(qc.pool);
        VK_CHECK(vkAllocateCommandBuffers(mDevice, &cbAllocInfo, &qc.cmd));
    };

    for (u64 i = 0; i < sFlightFrames; i++)
    {
        auto &fd = mFrames[i];
        initCommandsByFamily(fd.graphics, &mGraphics);
        initCommandsByFamily(fd.present, &mPresent);
        initCommandsByFamily(fd.compute, &mCompute);
        initCommandsByFamily(fd.transfer, &mTransfer);
    }
}

void Renderer::initDefaultRenderPass()
{
    // == ATTACHMENT(s) ==
    // att0 : Color
    VkAttachmentDescription color0  = {};
    color0.format                   = mSwapchainImageFormat;             // Copy swapchain format
    color0.samples                  = VK_SAMPLE_COUNT_1_BIT;             // 1 sample, no MSAA right now
    color0.loadOp                   = VK_ATTACHMENT_LOAD_OP_CLEAR;       // Clear on load
    color0.storeOp                  = VK_ATTACHMENT_STORE_OP_STORE;      // Keep stored on renderpass end
    color0.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // No stencil right now
    color0.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // No stencil right now
    color0.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;         // Let it as undefined on init
    color0.finalLayout              = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // Ready to display on renderpass end
    VkAttachmentReference refColor0 = {};
    refColor0.attachment            = 0;                                 // Attachment idx in the renderpass
    refColor0.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // att1 : Depth (@notsure: could not be also on 0????)
    VkAttachmentDescription depth0  = {};
    depth0.flags                    = 0;
    depth0.format                   = sDepthFormat;
    depth0.samples                  = VK_SAMPLE_COUNT_1_BIT;
    depth0.loadOp                   = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth0.storeOp                  = VK_ATTACHMENT_STORE_OP_STORE;
    depth0.stencilLoadOp            = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth0.stencilStoreOp           = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth0.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    depth0.finalLayout              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference refDepth0 = {};
    refDepth0.attachment            = 1;  // Attachment idx in the renderpass
    refDepth0.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // == SUBPASS(es) ==
    VkSubpassDescription subpass    = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;  // Create the renderpass for graphics
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &refColor0;
    subpass.pDepthStencilAttachment = &refDepth0;

    // == DEPENDENCIES ==
    // dep#1 : color0
    VkSubpassDependency depColor0 = {};
    depColor0.srcSubpass          = VK_SUBPASS_EXTERNAL;
    depColor0.dstSubpass          = 0;
    depColor0.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    depColor0.srcAccessMask       = 0;
    depColor0.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    depColor0.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // dep#2 : depth0
    VkSubpassDependency depDepth0 = {};
    depDepth0.srcSubpass          = VK_SUBPASS_EXTERNAL;
    depDepth0.dstSubpass          = 0;
    depDepth0.srcStageMask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depDepth0.srcAccessMask       = 0;
    depDepth0.dstStageMask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depDepth0.dstAccessMask       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // == RENDER PASS ==
    VkRenderPassCreateInfo renderpassCI = {};
    renderpassCI.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // att(s)
    auto const atts                     = std::array { color0, depth0 };
    renderpassCI.attachmentCount        = (u32)atts.size();
    renderpassCI.pAttachments           = atts.data();
    // subpass(es)
    renderpassCI.subpassCount           = 1;
    renderpassCI.pSubpasses             = &subpass;
    // dep(s)
    auto const deps                     = std::array { depColor0, depDepth0 };
    renderpassCI.dependencyCount        = (u32)deps.size();
    renderpassCI.pDependencies          = deps.data();

    VK_CHECK(vkCreateRenderPass(mDevice, &renderpassCI, nullptr, &mDefaultRenderPass));
    TO_DESTROY(vkDestroyRenderPass(mDevice, mDefaultRenderPass, nullptr));
}

void Renderer::initFramebuffers()
{
    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCI = {};
    framebufferCI.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCI.pNext                   = nullptr;
    framebufferCI.renderPass              = mDefaultRenderPass;
    framebufferCI.attachmentCount         = 1;
    framebufferCI.width                   = extentW();
    framebufferCI.height                  = extentH();
    framebufferCI.layers                  = 1;

    // Grab how many images we have in the swapchain
    mFramebuffers = decltype(mFramebuffers)(mSwapchainImages.size());

    // Create framebuffers for each of the swapchain image views
    for (size_t i = 0; i < mSwapchainImages.size(); i++)
    {
        auto const atts = std::array { mSwapchainImageViews[i], mDepthImageView };

        framebufferCI.attachmentCount = (u32)atts.size();
        framebufferCI.pAttachments    = atts.data();

        VK_CHECK(vkCreateFramebuffer(mDevice, &framebufferCI, nullptr, &mFramebuffers[i]));
        TO_DESTROY(vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr));
        TO_DESTROY(vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr));
    }
}

void Renderer::initSyncStructures()
{
    for (u64 i = 0; i < sFlightFrames; i++)
    {
        auto const fenceCI     = vk::CreateInfo::Fence();
        auto const semaphoreCI = vk::CreateInfo::Semaphore();
        auto      &fd          = mFrames[i];

        VK_CHECK(vkCreateFence(mDevice, &fenceCI, nullptr, &fd.renderFence));
        TO_DESTROY(vkDestroyFence(mDevice, fd.renderFence, nullptr));

        VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCI, nullptr, &fd.presentSemaphore));
        TO_DESTROY(vkDestroySemaphore(mDevice, fd.presentSemaphore, nullptr));

        VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCI, nullptr, &fd.renderSemaphore));
        TO_DESTROY(vkDestroySemaphore(mDevice, fd.renderSemaphore, nullptr));
    }
}

#define BTM_CREATE_VS(name)                                                                \
    auto vs_##name = vk::Create::ShaderModule(mDevice, #name, VK_SHADER_STAGE_VERTEX_BIT); \
    BTM_DEFER_(defer_vs_##name, vkDestroyShaderModule(mDevice, vs_##name, nullptr))

#define BTM_CREATE_FS(name)                                                                  \
    auto fs_##name = vk::Create::ShaderModule(mDevice, #name, VK_SHADER_STAGE_FRAGMENT_BIT); \
    BTM_DEFER_(defer_fs_##name, vkDestroyShaderModule(mDevice, fs_##name, nullptr))

#define BTM_CREATE_DRAW_SHADER(name) \
    BTM_CREATE_VS(name);             \
    BTM_CREATE_FS(name);

void Renderer::initMaterials()
{
    // Shaders
    BTM_CREATE_DRAW_SHADER(tri);
    BTM_CREATE_DRAW_SHADER(mesh);

    // Pipeline Layout(s)

    mPipelineLayouts = std::vector<VkPipelineLayout>(100, VK_NULL_HANDLE);

    auto const infoTri = vk::CreateInfo::PipelineLayout();
    VK_CHECK(vkCreatePipelineLayout(mDevice, &infoTri, nullptr, &mPipelineLayouts[0]));

    VkPushConstantRange push_constant = { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrices) };
    auto                infoMesh      = vk::CreateInfo::PipelineLayout();
    infoMesh.pPushConstantRanges      = &push_constant;
    infoMesh.pushConstantRangeCount   = 1;

    VK_CHECK(vkCreatePipelineLayout(mDevice, &infoMesh, nullptr, &mPipelineLayouts[1]));

    TO_DESTROY(for (auto L : mPipelineLayouts) if (L) vkDestroyPipelineLayout(mDevice, L, nullptr));

    //=====

    PipelineBuilder pb;

    // Pipeline 1

    pb.shaderStages.push_back(vk::CreateInfo::PipelineShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vs_tri));
    pb.shaderStages.push_back(vk::CreateInfo::PipelineShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fs_tri));
    pb.vertexInputInfo      = vk::CreateInfo::VertexInputState();
    pb.inputAssembly        = vk::CreateInfo::InputAssembly();
    pb.viewport.x           = 0.0f;
    pb.viewport.y           = 0.0f;
    pb.viewport.width       = mViewportSize.x;
    pb.viewport.height      = mViewportSize.y;
    pb.viewport.minDepth    = 0.0f;
    pb.viewport.maxDepth    = 1.0f;
    pb.scissor.offset       = { 0, 0 };
    pb.scissor.extent       = extent2D();
    pb.rasterizer           = vk::CreateInfo::RasterizationState();
    pb.multisampling        = vk::CreateInfo::MultisamplingState();
    pb.colorBlendAttachment = vk::Blend::None;
    pb.pipelineLayout       = mPipelineLayouts[0];
    pb.depthStencil         = vk::CreateInfo::DepthStencil(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    mPipelines.push_back(vk::Create::Pipeline(pb, mDevice, mDefaultRenderPass));
    createMaterial(mPipelines.back(), mPipelineLayouts[0], "flat");

    // Pipeline 2

    pb.shaderStages.clear();

    pb.shaderStages.push_back(vk::CreateInfo::PipelineShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vs_mesh));
    pb.shaderStages.push_back(vk::CreateInfo::PipelineShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fs_mesh));
    pb.vertexInputInfo = vk::CreateInfo::VertexInputState(VertexInputDescription::get());
    pb.rasterizer      = vk::CreateInfo::RasterizationState(Cull::NONE);
    pb.multisampling   = vk::CreateInfo::MultisamplingState(Samples::_1);  // Must match with renderpass ...
    pb.pipelineLayout  = mPipelineLayouts[1];

    mPipelines.push_back(vk::Create::Pipeline(pb, mDevice, mDefaultRenderPass));
    createMaterial(mPipelines.back(), mPipelineLayouts[1], "default");

    TO_DESTROY(for (auto P : mPipelines) if (P) vkDestroyPipeline(mDevice, P, nullptr));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void Renderer::executeImmediately(VkCommandPool pool, VkQueue queue, const std::function<void(VkCommandBuffer cb)> &fn)
{
    // Allocate
    VkCommandBuffer cb;
    auto const      cbAllocInfo = AllocInfo::CommandBuffer(pool);
    vkAllocateCommandBuffers(mDevice, &cbAllocInfo, &cb);

    // Record
    VkCommandBufferBeginInfo cbBeginInfo {};
    cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cb, &cbBeginInfo);
    fn(cb);
    vkEndCommandBuffer(cb);

    // Submit
    VkSubmitInfo submitInfo {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cb;
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);  // There is some room to improve using 'vkWaitForFences'-logic

    // Free
    vkFreeCommandBuffers(mDevice, pool, 1, &cb);
}

//-----------------------------------------------------------------------------

void Renderer::initMeshes()  // todo : this have to come from user-land
{
#if 1
    auto const addMesh = [this](auto const &name, auto const &path) { mMeshMap[name] = createMesh(btm::parseGltf(path)); };

    addMesh("monkey", "./Assets/Geometry/suzanne_donut.glb");
    addMesh("cube", "./Assets/Geometry/cube2.glb");
#else
    auto const scenes = {
        runtime::exepath() + "/Assets/Geometry/suzanne_donut.glb",  //
        // "./Assets/Geometry/cube2.glb",          //
    };

    for (auto const &path : scenes)
    {
        auto const mg = createMesh(btm::parseGltf(path));
        mMeshes.insert(mMeshes.end(), mg.begin(), mg.end());
    }
#endif
}

//-----------------------------------------------------------------------------

AllocatedBuffer Renderer::createBuffer(u64 bytes, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, bool addToDelQueue)
{
    AllocatedBuffer b;

    VkBufferCreateInfo info = {};
    info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size               = bytes;
    info.usage              = usage;
    info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;  // only one queue at time

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_UNKNOWN;
    allocInfo.requiredFlags           = memProps;

    VK_CHECK(vmaCreateBuffer(mAllocator, &info, &allocInfo, &b.buffer, &b.allocation, nullptr));

    if (addToDelQueue)
        TO_DESTROY(vmaDestroyBuffer(mAllocator, b.buffer, b.allocation));

    return b;
}

//-----------------------------------------------------------------------------

AllocatedBuffer Renderer::createBufferStaging(void const *data, u64 bytes, VkBufferUsageFlags usage)
{
    AllocatedBuffer b;

    // Create host
    auto const hostUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto const hostProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto       hostBuff  = createBuffer(bytes, hostUsage, hostProps, false);

    // Create dev
    auto const devUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
    auto const devProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto       devBuff  = createBuffer(bytes, devUsage, devProps);

    // Populate host
    void *hostMap;
    vmaMapMemory(mAllocator, hostBuff.allocation, &hostMap);
    memcpy(hostMap, data, bytes);
    vmaUnmapMemory(mAllocator, hostBuff.allocation);

    // Populate dev from host
    executeImmediately(
      frame().transfer.pool,
      mTransfer.queue,
      [&](VkCommandBuffer cb)
      {
          VkBufferCopy const copyRegion { 0, 0, bytes };
          vkCmdCopyBuffer(cb, hostBuff.buffer, devBuff.buffer, 1, &copyRegion);
      });

    // Host is no longer needed
    vmaDestroyBuffer(mAllocator, hostBuff.buffer, hostBuff.allocation);

    return devBuff;
}

//-----------------------------------------------------------------------------

MeshGroup Renderer::createMesh(btm::MeshGroup const &meshes)
{
    MeshGroup mg;

    for (auto const &mesh : meshes)
    {
        auto const iCP = asCoolPtr(mesh.indices);
        auto const vCP = asCoolPtr(mesh.vertices);

        auto const bI = createBufferStaging(iCP.data, iCP.bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        auto const bV = createBufferStaging(vCP.data, vCP.bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        mg.push_back({ iCP.count, bI, bV });
    }

    return mg;
}

//-----------------------------------------------------------------------------

Material *Renderer::createMaterial(VkPipeline pipeline, VkPipelineLayout layout, std::string const &name)
{
    return &(mMatMap.insert({ name, Material { pipeline, layout } })).first->second;
}

//-----------------------------------------------------------------------------

void Renderer::initTestScene()
{
    auto &testScene = mScenes["test"];

    RenderObject ro { mesh0("monkey"), material("default") };
    testScene.push_back(ro);

    for (int x = -20; x <= 20; x++)
    {
        for (int y = -20; y <= 20; y++)
        {
            glm::mat4 translation = glm::translate(glm::mat4 { 1.0 }, glm::vec3(x, 0, y));
            glm::mat4 scale       = glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.2, 0.2, 0.2));
            ro.transform          = translation * scale;
            testScene.push_back(ro);
        }
    }
}

//-----------------------------------------------------------------------------

void Renderer::drawScene(std::string const &name, Camera const &cam)
{
    //===============

    glm::mat4 const &view       = cam.V();  // glm::translate(glm::mat4(1.f), { 0.f, 0.f, -4.f });
    glm::mat4 const &projection = cam.P();  // glm::perspective(glm::radians(70.f), mViewportSize.x / mViewportSize.y, 0.1f, 200.0f);

    Matrices constants;
    // constants.N   = glm::transpose(glm::inverse(model));
    // constants.MVP = projection * view * model;
    // vkCmdPushConstants(mGraphicsCB, mPipelineLayouts[1], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrices), &constants);

    //===============

    bool const  sceneExists = mScenes.count(name) > 0;
    auto const &scene       = sceneExists ? mScenes[name] : std::vector<RenderObject> {};
    // BTM_INFOF("SCENE: IS VALID? {}, IS EMPTY? {}", sceneExists, scene.empty());

    //===============

    Mesh     *lastMesh     = nullptr;
    Material *lastMaterial = nullptr;

    for (auto const &ro : scene)
    {
        // update push-constant
        constants.N   = glm::transpose(glm::inverse(ro.transform));
        constants.MVP = projection * view * ro.transform;
        vkCmdPushConstants(frame().graphics.cmd, mPipelineLayouts[1], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrices), &constants);
        // only bind the pipeline if it doesn't match with the already bound one
        if (ro.material != lastMaterial)
        {
            ro.material->bind(frame().graphics.cmd);
            lastMaterial = ro.material;
        }
        // only bind the mesh if it's a different one from last bind
        if (ro.mesh != lastMesh)
        {
            ro.mesh->bind(frame().graphics.cmd);
            lastMesh = ro.mesh;
        }
        // draw
        ro.mesh->draw(frame().graphics.cmd);
    }
}

//-----------------------------------------------------------------------------

}  // namespace btm::vk
