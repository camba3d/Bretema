#include "vk_renderer.hpp"
#include "vk_init.hpp"

#include <chrono>

namespace btm::vk
{

constexpr int BTM_VK_MAJOR_VERSION = 1;
constexpr int BTM_VK_MINOR_VERSION = 2;
#define BTM_VK_VER BTM_VK_MAJOR_VERSION, BTM_VK_MINOR_VERSION

//-----------------------------------------------------------------------------

Renderer::Renderer(Ref<btm::Window> window) : btm::BaseRenderer(window)
{
    // * https://github.com/charles-lunarg/vk-bootstrap/blob/master/docs/getting_started.md

    initVulkan();
    initSwapchain();
    initDefaultRenderPass();
    initFramebuffers();
    initCommands();
    initSyncStructures();
    initPipelines();

    // init_descriptors();
    // init_pipelines();
    // load_images();
    loadMeshes();
    // init_scene();

    markAsInit();
}

void Renderer::draw()
{
    auto cmd = mMainCommandBuffer;

    // Wait for GPU (1 second timeout)
    VK_CHECK(vkWaitForFences(mDevice, 1, &mRenderFence, true, sOneSec));
    VK_CHECK(vkResetFences(mDevice, 1, &mRenderFence));

    // Request image from the swapchain (1 second timeout)
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapchain, sOneSec, mPresentSemaphore, nullptr, &swapchainImageIndex));

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
    renderpassBI.renderArea.extent     = extent();
    renderpassBI.framebuffer           = mFramebuffers[swapchainImageIndex];
    renderpassBI.clearValueCount       = 1;
    renderpassBI.pClearValues          = &clear;

    vkCmdBeginRenderPass(cmd, &renderpassBI, VK_SUBPASS_CONTENTS_INLINE);

    //===========

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines[1]);

    // bind the mesh vertex buffer with offset 0
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &mMeshes[0].vertices.buffer, &offset);

    // set push constant MVP
    glm::vec3 camPos     = { 0.f, 0.f, -4.f };
    glm::mat4 view       = glm::translate(glm::mat4(1.f), camPos);
    glm::mat4 projection = glm::perspective(glm::radians(70.f), mViewportSize.x / mViewportSize.y, 0.1f, 200.0f);
    // projection[1][1] *= -1;  // ??
    glm::mat4 model      = glm::rotate(glm::mat4(1.f), glm::radians(mFrameNumber * 0.4f), glm::vec3(0, 1, 0));
    MeshPushConstants constants;
    constants.modelViewProj = projection * view * model;
    // upload the matrix to the GPU via push constants
    vkCmdPushConstants(cmd, mPipelineLayouts[1], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

    // we can now draw the mesh
    vkCmdDraw(cmd, mMeshes[0].vertexCount, 1, 0, 0);

    //===========

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

void Renderer::cleanup()
{
    if (!isInitialized())
        return;

    vkWaitForFences(mDevice, 1, &mRenderFence, true, sOneSec);  // make sure the GPU has stopped doing its things

    mMainDelQueue.flush();

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
    mGraphicsQ = { vkbDevice, vkb::QueueType::graphics };
    mComputeQ  = { vkbDevice, vkb::QueueType::compute };
    mPresentQ  = { vkbDevice, vkb::QueueType::present };
    mTransferQ = { vkbDevice, vkb::QueueType::transfer };
}

void Renderer::initSwapchain()
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
    mMainDelQueue.push_back([this]() { vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr); });
}

void Renderer::initCommands()
{
    // Create a command pool for commands submitted to the graphics queue
    auto const cmdPoolFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    auto const cmdPoolCI    = vk::CreateInfo::CommandPool(mGraphicsQ.family, cmdPoolFlags);
    VK_CHECK(vkCreateCommandPool(mDevice, &cmdPoolCI, nullptr, &mCommandPool));

    // Allocate the default command buffer that we will use for rendering
    auto const cmdAllocInfo = vk::AllocInfo::CommandBuffer(mCommandPool);
    VK_CHECK(vkAllocateCommandBuffers(mDevice, &cmdAllocInfo, &mMainCommandBuffer));

    mMainDelQueue.push_back([this]() { vkDestroyCommandPool(mDevice, mCommandPool, nullptr); });
}

void Renderer::initDefaultRenderPass()
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

    // RenderPass deletion-queue
    mMainDelQueue.push_back([this]() { vkDestroyRenderPass(mDevice, mDefaultRenderPass, nullptr); });
}

void Renderer::initFramebuffers()
{
    auto const lExtent = extent();

    // Create the framebuffers for the swapchain images.
    // This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCI = {};
    framebufferCI.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCI.pNext                   = nullptr;
    framebufferCI.renderPass              = mDefaultRenderPass;
    framebufferCI.attachmentCount         = 1;
    framebufferCI.width                   = lExtent.width;
    framebufferCI.height                  = lExtent.height;
    framebufferCI.layers                  = 1;

    // Grab how many images we have in the swapchain
    mFramebuffers = decltype(mFramebuffers)(mSwapchainImages.size());

    // Create framebuffers for each of the swapchain image views
    for (size_t i = 0; i < mSwapchainImages.size(); i++)
    {
        framebufferCI.pAttachments = &mSwapchainImageViews[i];
        VK_CHECK(vkCreateFramebuffer(mDevice, &framebufferCI, nullptr, &mFramebuffers[i]));

        // deletion-queue
        mMainDelQueue.push_back(
          [this, i]()
          {
              // Framebuffer
              vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr);
              // SwapchainIamgeView
              vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
          });
    }
}

void Renderer::initSyncStructures()
{
    auto const fenceCI = vk::CreateInfo::Fence();
    VK_CHECK(vkCreateFence(mDevice, &fenceCI, nullptr, &mRenderFence));

    auto const semaphoreCI = vk::CreateInfo::Semaphore();
    VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCI, nullptr, &mPresentSemaphore));
    VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCI, nullptr, &mRenderSemaphore));

    // deletion-queue
    mMainDelQueue.push_back(
      [this]()
      {
          // Fence(s)
          vkDestroyFence(mDevice, mRenderFence, nullptr);

          // Semaphore(s)
          vkDestroySemaphore(mDevice, mPresentSemaphore, nullptr);
          vkDestroySemaphore(mDevice, mRenderSemaphore, nullptr);
      });
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

void Renderer::initPipelines()
{
    // Shaders
    BTM_CREATE_DRAW_SHADER(tri);
    BTM_CREATE_DRAW_SHADER(mesh);

    // Pipeline Layout(s)

    mPipelineLayouts = std::vector<VkPipelineLayout>(100, VK_NULL_HANDLE);

    auto const infoTri = vk::CreateInfo::PipelineLayout();
    VK_CHECK(vkCreatePipelineLayout(mDevice, &infoTri, nullptr, &mPipelineLayouts[0]));

    auto infoMesh = vk::CreateInfo::PipelineLayout();

    VkPushConstantRange push_constant = { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants) };
    infoMesh.pPushConstantRanges      = &push_constant;
    infoMesh.pushConstantRangeCount   = 1;

    VK_CHECK(vkCreatePipelineLayout(mDevice, &infoMesh, nullptr, &mPipelineLayouts[1]));

    // deletion-queue :: @note might this be deleted after pipelines ??
    mMainDelQueue.push_back(
      [this]()
      {
          for (auto layout : mPipelineLayouts)
              if (layout != VK_NULL_HANDLE)
                  vkDestroyPipelineLayout(mDevice, layout, nullptr);
      });

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
    pb.scissor.extent       = extent();
    pb.rasterizer           = vk::CreateInfo::RasterizationState();
    pb.multisampling        = vk::CreateInfo::MultisamplingState();
    pb.colorBlendAttachment = vk::Blend::None;
    pb.pipelineLayout       = mPipelineLayouts[0];

    mPipelines.push_back(vk::Create::Pipeline(pb, mDevice, mDefaultRenderPass));

    // Pipeline 2

    pb.shaderStages.clear();

    pb.shaderStages.push_back(vk::CreateInfo::PipelineShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vs_mesh));
    pb.shaderStages.push_back(vk::CreateInfo::PipelineShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fs_mesh));
    pb.vertexInputInfo = vk::CreateInfo::VertexInputState(VertexInputDescription::get());
    pb.rasterizer      = vk::CreateInfo::RasterizationState(btm::Cull::NONE);
    pb.multisampling   = vk::CreateInfo::MultisamplingState(btm::Samples::_1);  // Must match with renderpass ...
    pb.pipelineLayout  = mPipelineLayouts[1];

    mPipelines.push_back(vk::Create::Pipeline(pb, mDevice, mDefaultRenderPass));

    // deletion-queue
    mMainDelQueue.push_back(
      [this]()
      {
          for (auto pipeline : mPipelines)
              vkDestroyPipeline(mDevice, pipeline, nullptr);
      });
}

//-----------------------------------------------------------------------------

void Renderer::loadMeshes()
{
    btm::Mesh mesh;

    mesh.vertices.resize(3);

    mesh.vertices[0].pos = { 1.f, 1.f, 0.f };
    mesh.vertices[1].pos = { -1.f, 1.f, 0.f };
    mesh.vertices[2].pos = { 0.f, -1.f, 0.f };

    // verts[0].color = Color::Orange;
    // verts[1].color = Color::StrongYellow;
    // verts[2].color = Color::Yellow;

    // auto const gltf = btm::parseGltf("./Assets/Geometry/default_scene_A.gltf");  // @HERE!

    // mMeshes.push_back(createMesh(mesh.vertices));

    auto meshes = btm::parseGltf("./Assets/Geometry/default_scene_A.gltf");
    mMeshes.push_back(createMesh(meshes.at(0).vertices));
}

//-----------------------------------------------------------------------------

Mesh Renderer::createMesh(btm::Vertices const &verts)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size               = verts.size() * sizeof(btm::Mesh::Vertex);  // bytes
    bufferInfo.usage              = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage                   = VMA_MEMORY_USAGE_CPU_TO_GPU;  //@deprecated ??

    Mesh mesh;
    mesh.vertexCount = verts.size();

    // Indices
    //...@todo

    // Vertices
    VK_CHECK(vmaCreateBuffer(
      mAllocator,
      &bufferInfo,
      &vmaallocInfo,
      &mesh.vertices.buffer,
      &mesh.vertices.allocation,
      nullptr));

    void *data;
    vmaMapMemory(mAllocator, mesh.vertices.allocation, &data);
    memcpy(data, verts.data(), verts.size() * sizeof(btm::Mesh::Vertex));
    vmaUnmapMemory(mAllocator, mesh.vertices.allocation);

    // deletion-queue
    mMainDelQueue.push_back([=, this]()
                            { vmaDestroyBuffer(mAllocator, mesh.vertices.buffer, mesh.vertices.allocation); });

    return mesh;
}

//-----------------------------------------------------------------------------

}  // namespace btm::vk
