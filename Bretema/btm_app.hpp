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
    static auto constexpr sDefaultState = Input::State::Release;

public:
    App(std::string name, RenderAPI renderAPI);
    void run();
    void cleanup();

    void reset();

    std::string name() const;

    void markToClose();
    bool isMarkedToClose() const;

    glm::vec2 cursor() const;
    void      cursor(glm::vec2 cursor);

    Input::State key(Input::Key k) const;
    void         key(Input::Key k, Input::State s);

    Input::State mouse(Input::Mouse m) const;
    void         mouse(Input::Mouse m, Input::State s);

private:
    std::string mName      = "";
    RenderAPI   mRenderAPI = RenderAPI::Vulkan;

    bool mInit  = false;
    bool mClose = false;

    glm::vec2                        mCursor = { 0, 0 };
    umap<Input::Key, Input::State>   mKeys   = {};
    umap<Input::Mouse, Input::State> mMouse  = {};

    Ref<btm::Window>   mMainWindow = nullptr;
    btm::BaseRenderer *mRenderer   = nullptr;
};

}  // namespace btm