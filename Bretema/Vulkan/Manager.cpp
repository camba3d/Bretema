#pragma once

#include "Manager.hpp"

#include "../Window.hpp"

namespace btm::vk
{

constexpr int BTM_VK_MAJOR_VERSION = 1;
constexpr int BTM_VK_MINOR_VERSION = 2;

void Manager::initialize(std::vector<> windowHandles)
{
    // * https://github.com/charles-lunarg/vk-bootstrap/blob/master/docs/getting_started.md

    BTM_ASSERT_X(!windowHandles.empty(), "At least one window must be provided.");

    // vkbootstrap : Create a instance with some setup
    vkb::InstanceBuilder instanceBuilder;
    auto                 instanceBuilderResult = instanceBuilder //
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
    glfwCreateWindowSurface(mInstance, (GLFWwindow *)windowHandles[0], nullptr, &mSurface);

    // vkbootstrap : Select a GPU based on some criteria
    vkb::PhysicalDeviceSelector physicalDeviceSelector{instanceVKB};
    vkb::PhysicalDevice         physicalDeviceVKB = physicalDeviceSelector //
                                                .set_minimum_version(BTM_VK_MAJOR_VERSION, BTM_VK_MINOR_VERSION)
                                                .set_surface(mSurface)
                                                .select()
                                                .value();

    // vkbootstrap : Create the final Vulkan device
    vkb::DeviceBuilder deviceBuilder{physicalDeviceVKB};
    vkb::Device        deviceVKB = deviceBuilder.build().value();

    // store : DEVICE
    mDevice    = deviceVKB.device;
    // store : GPU (PHYSICAL DEVICE)
    mChosenGPU = physicalDeviceVKB.physical_device;

    mIsInitialized = true;
}

void Manager::createSwapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{mChosenGPU, mDevice, mSurface};

    vkb::Swapchain swapchainVKB = swapchainBuilder
                                      .use_default_format_selection()
                                      // use vsync present mode
                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                      .set_desired_extent(btm::Window::size())
                                      .build()
                                      .value();

    // store swapchain and its related images
    _swapchain           = vkbSwapchain.swapchain;
    _swapchainImages     = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();

    _swapchainImageFormat = vkbSwapchain.image_format;
}

} // namespace btm::vk
