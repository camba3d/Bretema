#pragma once

#include "Vk/vk_renderer.hpp"

#include "btm_base.hpp"
#include "btm_renderer.hpp"
#include "btm_camera.hpp"

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
    // LIFETIME
    App(std::string name, RenderAPI renderAPI);

    // ACTIONS
    void run();
    void reset();
    void cleanup();
    void markToClose();

    // PROPERTIES
    // --- Read Only
    std::string name() const;
    bool        isMarkedToClose() const;

private:
    // FRIENDS
    friend class Window;

    // PROPERTIES : Read-Only setters
    void cursor(glm::vec2 cursor);
    void key(UI::Key k, UI::State s);
    void mouse(UI::Mouse m, UI::State s);
    void wheel(glm::vec2 wheel);

    // METHODS
    UI::Info genInputInfo(glm::vec2 displ = ZERO2, glm::vec2 wheel = ZERO2);
    void     onInputChange(UI::Info const &ui);

    // PROPERTIES VARS
    std::string    mName   = "";
    glm::vec2      mCursor = { 0, 0 };
    UI::KeyState   mKeys   = {};
    UI::MouseState mMouse  = {};

    // INSTANCE VARS
    bool               mInit       = false;
    bool               mClose      = false;
    RenderAPI          mRenderAPI  = RenderAPI::Vulkan;
    sPtr<btm::Window>  mMainWindow = nullptr;
    btm::BaseRenderer *mRenderer   = nullptr;

    std::vector<Camera> mCameras = { sDefaultCamera };

    inline static Camera const sDefaultCamera { "Main" };
};

}  // namespace btm