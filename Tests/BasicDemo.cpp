#include "Bretema/Vk/vk_base.hpp"
#include "Bretema/Vk/vk_engine.hpp"
#include "Bretema/btm_base.hpp"
#include "Bretema/btm_window.hpp"

#include <thread>

int main()
{
    // Create windows
    std::vector<btm::Window> windows;
    for (size_t i = 0; i < 1; ++i)
        windows.emplace_back(1920, 1080, fmt::format("Main Window {}", i));

    auto const &mainWindow = windows.at(0);

    // Init render engine
    btm::vk::Engine vkEngine;
    vkEngine.initialize(mainWindow.handle(), mainWindow.size());

    // Run "game-loop"
    bool isAnyWindowOpen = true;

    while (isAnyWindowOpen)
    {
        isAnyWindowOpen = false;
        for (auto &window : windows)
        {
            bool const shouldClose = window.shouldClose();

            // render...
            vkEngine.draw();

            if (shouldClose)
                window.destroy();  // this may need to trigger a call on vkEngine...

            isAnyWindowOpen |= !shouldClose;
        }

        btm::Window::waitEvents();
    }

    // Terminate render engine and windows
    vkEngine.cleanup();
    btm::Window::terminate();
    return 0;
}