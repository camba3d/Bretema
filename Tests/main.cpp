#include "Bretema/bm/app.hpp"
#include "Bretema/bm/BuildInfo.hpp"

BM_FORCE_DISCRETE_GPU;

int main(int argc, char *argv[])
{
    BM_INFOF("Initializing Bretema Engine #{}.{}", bm::BuildInfo.version.major, bm::BuildInfo.version.minor);

    bm::App app { "Bretema", bm::RenderAPI::Vulkan };

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

    BM_INFOF("Closing Bretema Engine #{}.{}", bm::BuildInfo.version.major, bm::BuildInfo.version.minor);

    return 0;
}