#include "Bretema/Base.hpp"
#include "Bretema/Window.hpp"

#include "Bretema/Vulkan/Base.hpp"

#include <thread>

int main()
{
    std::vector<btm::Window> windows;

    for (size_t i = 0; i < 1; ++i)
        windows.emplace_back(1920, 1080, fmt::format("Main Window {}", i));

    bool isAnyWindowOpen = true;
    while (isAnyWindowOpen)
    {
        isAnyWindowOpen = false;
        for (auto &window : windows)
        {
            bool const shouldClose = window.shouldClose();

            if (shouldClose)
                window.destroy();

            isAnyWindowOpen |= !shouldClose;
        }

        btm::Window::pollEvents();
    }

    btm::Window::terminate();
    return 0;
}