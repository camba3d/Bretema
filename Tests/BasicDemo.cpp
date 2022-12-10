#include "Bretema/Base.hpp"
#include "Bretema/Vulkan/Base.hpp"
#include "Bretema/Vulkan/Manager.hpp"
#include "Bretema/Window.hpp"

#include <thread>

int main()
{
    // Create windows
    std::vector<btm::Window> windows;
    for (size_t i = 0; i < 1; ++i)
        windows.emplace_back(1920, 1080, fmt::format("Main Window {}", i));

    auto const &mainWindow = windows.at(0);

    // Init render engine
    btm::vk::Manager vkManager;
    vkManager.initialize(mainWindow.handle(), mainWindow.size());

    // Run game loop
    bool isAnyWindowOpen = true;

    while (isAnyWindowOpen)
    {
        isAnyWindowOpen = false;
        for (auto &window : windows)
        {
            bool const shouldClose = window.shouldClose();

            if (shouldClose)
                window.destroy();  // this may need to trigger a call on vkManager...

            isAnyWindowOpen |= !shouldClose;
        }

        btm::Window::waitEvents();
    }

    // Terminate render engine and windows
    vkManager.cleanup();
    btm::Window::terminate();
    return 0;
}