#include "Bretema/btm_app.hpp"

int main()
{
    btm::App app { "Bretema", btm::RenderAPI::Vulkan };
    while (true)  // Allows app restart
    {
        app.run();
        app.cleanup();
        if (app.isMarkedToClose())
            break;
        app.reset();
    }

    return 0;
}
