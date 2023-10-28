#pragma once

#include "Vk/vk_renderer.hpp"

#include "btm_base.hpp"
#include "btm_renderer.hpp"
#include "btm_camera.hpp"
#include "btm_userInput.hpp"

namespace btm
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

    sPtr<btm::Window>  mMainWindow = nullptr;
    btm::BaseRenderer *mRenderer   = nullptr;
    RenderAPI          mRenderAPI  = RenderAPI::Vulkan;

    std::vector<Camera> mCameras = { sDefaultCamera };
    btm::Timer_Ms       mETimer  = { "MainLoop" };

    // UserInput mUserInput = { nullptr };
    UserInput mUserInput = { [this](UserInput *ui)
                             {
                                 if (ui)
                                     for (auto &camera : mCameras) camera.onInputChange(*ui);
                             } };

    inline static Camera const sDefaultCamera { "Main" };
};

}  // namespace btm