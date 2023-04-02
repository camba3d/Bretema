#pragma once

#include "Vk/vk_renderer.hpp"

#include "btm_base.hpp"
#include "btm_renderer.hpp"

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
    std::string  name() const;
    bool         isMarkedToClose() const;
    glm::vec2    cursor() const;
    Input::State key(Input::Key k) const;
    Input::State mouse(Input::Mouse m) const;

private:
    // FRIENDS
    friend class Window;

    // PROPERTIES : Read-Only private Write(s)
    void cursor(glm::vec2 cursor);
    void key(Input::Key k, Input::State s);
    void mouse(Input::Mouse m, Input::State s);

    // PROPERTIES VARS
    std::string                      mName   = "";
    glm::vec2                        mCursor = { 0, 0 };
    umap<Input::Key, Input::State>   mKeys   = {};
    umap<Input::Mouse, Input::State> mMouse  = {};

    // INSTANCE VARS
    bool               mInit       = false;
    bool               mClose      = false;
    RenderAPI          mRenderAPI  = RenderAPI::Vulkan;
    Ref<btm::Window>   mMainWindow = nullptr;
    btm::BaseRenderer *mRenderer   = nullptr;

    // STATIC VARS
    static auto constexpr sDefaultState = Input::State::Release;
};

}  // namespace btm