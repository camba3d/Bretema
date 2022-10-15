#include "Bretema/Base.hpp"
#include "Bretema/Window.hpp"

#include <thread>

int main()
{
#if 0
    auto        mainWindow = btm::Window(1920, 1080, "Main Window");
    while (!mainWindow.isMarkedToClose())
    {
        mainWindow.pollEvents();
    }
    mainWindow.destroy();
    mainWindow.terminate();
#else
    std::vector<btm::Window> windows;

    for (size_t i = 0; i < 3; ++i)
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
#endif

    return 0;
}