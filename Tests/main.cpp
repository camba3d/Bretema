#include "Bretema/btm_app.hpp"

int main()
{
    btm::parseGltf("./Assets/Geometry/default_scene_A.gltf");
    btm::parseGltf("./Assets/Geometry/default_scene_B.gltf");
    btm::parseGltf("./Assets/Geometry/default_scene_C.glb");

    // return 0;

    btm::App bretema { "Bretema", btm::RenderAPI::Vulkan };
    while (true)  // Allows app restart
    {
        bretema.run();
        bretema.cleanup();
        if (bretema.isMarkedToClose())
            break;
        bretema.reset();
    }

    return 0;
}
