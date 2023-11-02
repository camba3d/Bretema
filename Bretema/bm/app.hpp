#pragma once

#include "../vk/renderer.hpp"

#include "base.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "userInput.hpp"

namespace bm
{

enum RenderAPI
{
    Vulkan,
    // WebGpu,
    // Metal,
    // DX12,
    // OpenGL,
};

class App
{
public:
    App(std::string name, RenderAPI renderAPI);

    void run();
    void reset();
    void cleanup();
    void markToClose();

    std::string name() const;
    bool        isMarkedToClose() const;

private:
    friend class Window;

    void onInputChange();

    std::string mName = "";

    bool mInit  = false;
    bool mClose = false;

    sPtr<bm::Window>  mMainWindow = nullptr;
    bm::BaseRenderer *mRenderer   = nullptr;
    RenderAPI         mRenderAPI  = RenderAPI::Vulkan;

    std::vector<Camera> mCameras = { sDefaultCamera };
    bm::Timer_Ms        mETimer  = { "MainLoop" };

    // UserInput mUserInput = { nullptr };
    UserInput mUserInput = { [this](UserInput *ui)
                             {
                                 if (ui)
                                     for (auto &camera : mCameras) camera.onInputChange(*ui);
                             } };

    inline static Camera const sDefaultCamera { "Main" };
};

}  // namespace bm