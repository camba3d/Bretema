#include "Bretema/btm_app.hpp"
#include "Bretema/BuildInfo.hpp"

BTM_FORCE_DISCRETE_GPU;

int main(int argc, char *argv[])
{
    BTM_INFOF("Initializing Bretema Engine #{}.{}", btm::BuildInfo.version.major, btm::BuildInfo.version.minor);

    btm::App app { "Bretema", btm::RenderAPI::Vulkan };

    while (true)
    {
        app.run();
        app.cleanup();

        if (app.isMarkedToClose())
        {
            break;
        }

        app.reset();
    }

    BTM_INFOF("Closing Bretema Engine #{}.{}", btm::BuildInfo.version.major, btm::BuildInfo.version.minor);

    return 0;
}