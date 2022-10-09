#include "Bretema/Base.hpp"
#include "Bretema/Window.hpp"

int main()
{
    auto mainWindow = btm::Window(1920, 1080, "Main Window");

    while (!mainWindow.isMarkedToClose())
    {
        mainWindow.pollEvents();
    }

    mainWindow.destroy();
    mainWindow.terminate();

    return 0;
}