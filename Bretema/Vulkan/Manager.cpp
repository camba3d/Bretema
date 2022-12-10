#include "Manager.hpp"

#include "../Window.hpp"
#include "Initializers.hpp"

namespace btm::vk
{

constexpr int BTM_VK_MAJOR_VERSION = 1;
constexpr int BTM_VK_MINOR_VERSION = 2;

void Manager::initialize(void *windowHandle, glm::vec2 const &viewportSize)
{
    // * https://github.com/charles-lunarg/vk-bootstrap/blob/master/docs/getting_started.md

    BTM_ASSERT_X(windowHandle, "Invalid window handle");
    BTM_ASSERT_X(viewportSize.x > 0 && viewportSize.y > 0, "Invalid viewport size");

    // vkbootstrap : Create a instance with some setup
    vkb::InstanceBuilder instanceBuilder;
    auto                 instanceBuilderResult = instanceBuilder  //
                                   .set_app_name("Bretema Default Engine")
                                   .request_validation_layers(true)
                                   .require_api_version(BTM_VK_MAJOR_VERSION, BTM_VK_MINOR_VERSION, 0)
                                   .use_default_debug_messenger()
                                   .build();
    vkb::Instance instanceVKB = instanceBuilderResult.value();

    // store : INSTACE
    mInstance       = instanceVKB.instance;
    // store : DEBUG MESSENGER
    mDebugMessenger = instanceVKB.debug_messenger;

    // store : SURFACE
    glfwCreateWindowSurface(mInstance, (GLFWwindow *)windowHandle, nullptr, &mSurface);

    // vkbootstrap : Select a GPU based on some criteria
    vkb::PhysicalDeviceSelector physicalDeviceSelector { instanceVKB };
    vkb::PhysicalDevice         physicalDeviceVKB = physicalDeviceSelector  //
                                              .set_minimum_version(BTM_VK_MAJOR_VERSION, BTM_VK_MINOR_VERSION)
                                              .set_surface(mSurface)
                                              .select()
                                              .value();

    // vkbootstrap : Create the final Vulkan device
    vkb::DeviceBuilder deviceBuilder { physicalDeviceVKB };
    vkb::Device        deviceVKB = deviceBuilder.build().value();

    // store : DEVICE
    mDevice    = deviceVKB.device;
    // store : GPU (PHYSICAL DEVICE)
    mChosenGPU = physicalDeviceVKB.physical_device;

    // vkbootstrap : Get queues
    mGraphicsQueue       = deviceVKB.get_queue(vkb::QueueType::graphics).value();
    mGraphicsQueueFamily = deviceVKB.get_queue_index(vkb::QueueType::graphics).value();

    // create : SWAPCHAIN
    createSwapchain(viewportSize);

    // create : COMMANDS
    createCommands();

    mIsInitialized = true;
}

void Manager::createSwapchain(glm::vec2 const &viewportSize)
{
    BTM_ASSERT_X(viewportSize.x > 0 && viewportSize.y > 0, "Invalid viewport size");

    // vkbootstrap : Create swapchain
    vkb::SwapchainBuilder swapchainBuilder { mChosenGPU, mDevice, mSurface };
    vkb::Swapchain        swapchainVKB = swapchainBuilder.use_default_format_selection()
                                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)  // fifo = vsync present mode
                                    .set_desired_extent(viewportSize.x, viewportSize.y)
                                    .set_desired_min_image_count(3)
                                    .build()
                                    .value();

    // store : swapchain and its related images
    mSwapchain            = swapchainVKB.swapchain;
    mSwapchainImages      = swapchainVKB.get_images().value();
    mSwapchainImageViews  = swapchainVKB.get_image_views().value();
    mSwapchainImageFormat = swapchainVKB.image_format;
}

void Manager::createCommands()
{
    // Create a command pool for commands submitted to the graphics queue

    auto const cmdPoolFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    auto const cmdPoolCI    = init::cmdPoolCreateInfo(mGraphicsQueueFamily, cmdPoolFlags);
    BTM_VK_CHECK(vkCreateCommandPool(mDevice, &cmdPoolCI, nullptr, &mCommandPool));

    // Allocate the default command buffer that we will use for rendering

    auto const cmdAllocInfo = init::cmdBufferAllocInfo(mCommandPool);
    BTM_VK_CHECK(vkAllocateCommandBuffers(mDevice, &cmdAllocInfo, &mMainCommandBuffer));
}

void Manager::cleanup()
{
    if (!mIsInitialized)
        return;

    // Commands
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

    // Swapchain resources

    for (int i = mSwapchainImageViews.size() - 1; i >= 0; --i)
        if (mSwapchainImageViews[i])
            vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);

    // WARNING : this is triggering validation layers
    // for (int i = mSwapchainImages.size() - 1; i >= 0; --i)
    //     if (mSwapchainImages[i])
    //         vkDestroyImage(mDevice, mSwapchainImages[i], nullptr);

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
