#include "Bretema/btm_app.hpp"

int main()
{
    // btm::parseGltf("./Assets/Geometry/default_scene_A.gltf");
    // btm::parseGltf("./Assets/Geometry/default_scene_B.gltf");
    // btm::parseGltf("./Assets/Geometry/default_scene_C.glb");

    // return 0;
    //

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
