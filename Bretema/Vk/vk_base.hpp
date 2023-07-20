#pragma once

// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/WSIheaders.html

#if __APPLE__
#    include "TargetConditionals.h"
#    if TARGET_OS_OSX == 1
#        define VK_USE_PLATFORM_MACOS_MVK
#    elif TARGET_OS_IPHONE == 1 || TARGET_OS_SIMULATOR == 1
#        define VK_USE_PLATFORM_IOS_MVK
#    endif
#elif _WIN64 || _WIN32
#    define VK_USE_PLATFORM_WIN32_KHR
#elif __linux || __unix || __posix
// TODO: Get specific platform based on server
#    define VK_USE_PLATFORM_XLIB_KHR
// #define VK_USE_PLATFORM_XCB_KHR
// #define VK_USE_PLATFORM_WAYLAND_KHRM
// #define VK_USE_PLATFORM_MIR_KHR
// #define VK_USE_PLATFORM_XLIB_XRANDR_EXT
#elif __ANDROID__
#    define VK_USE_PLATFORM_ANDROID_KHR
#endif

#define VK_ENABLE_BETA_EXTENSIONS
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

//===[MACROS]======

// #define FADE_VK_INSTANCE_LAYERS     "VK_LAYER_KHRONOS_validation"
// #define FADE_VK_INSTANCE_EXTENSIONS VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME

// #if __APPLE__
// #define FADE_VK_GPU_EXTENSIONS "VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME
// #else
// #define FADE_VK_GPU_EXTENSIONS VK_KHR_SWAPCHAIN_EXTENSION_NAME
// #endif

// . Validate api calls
#define VK_CHECK(x)                                                                                      \
    do {                                                                                                 \
        if (VkResult res = x; res != VK_SUCCESS) BTM_ABORT("{} : {}", btm::vk::str::Result.at(res), #x); \
    } while (0)

#define VKB_CHECK(x)                                                                      \
    do {                                                                                  \
        if (!x.has_value())                                                               \
            if (auto err = x.error().message(); err != "") BTM_ABORT("{} : {}", err, #x); \
    } while (0)

// . Get instance functions
#define VK_INSTANCE_FN(instance, extName, ...)                                                                   \
    do {                                                                                                         \
        if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) fn(instance, __VA_ARGS__); \
        else BTM_ERRF("Function {} is not available", #extName);                                                 \
    } while (0)

namespace btm::vk
{

}  // namespace btm::vk