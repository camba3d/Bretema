#pragma once

#include "Vk/vk_renderer.hpp"

#include "btm_base.hpp"
#include "btm_renderer.hpp"

namespace btm
{

enum RenderSelector
{
    UseVulkan,
    // UseWebGpu,
    // UseMetal,
    // UseDX12,
    // UseOpenGL,
};

struct Globals_
{
    inline static auto SelectedRenderer = UseVulkan;
} inline static Globals {};

// clang-format off
#define RENDERER(call)                                                                    \
    do                                                                                    \
    {                                                                                     \
        BTM_ASSERT(mRenderer);                                                            \
        switch (Globals.SelectedRenderer)                                                 \
        {                                                                                 \
            case UseVulkan: static_cast<vk::Renderer *>(mRenderer)->call; break;      \
            default: BTM_ABORT("Selected renderer is not implemented yet!"); break;       \
        }                                                                                 \
    } while (0);
// clang-format on

class App
{
    static auto constexpr sDefaultState = Input::State::Release;

public:
    App(std::string name) : mName(name)
    {
        BTM_ABORTF_IF(
          mMainWindow || mRenderer,
          "Initialization fiasco... Window({}) or Renderer({}) already exists!",
          BTM_PTRSTR(mMainWindow),
          BTM_PTRSTR(mRenderer));

        // Init window
        mMainWindow = MakeRef<Window>(1920, 1080, "Default Window", this);

        // Init renderer
        switch (Globals.SelectedRenderer)
        {
            case UseVulkan: mRenderer = new vk::Renderer(mMainWindow); break;
            default: BTM_ABORT("Selected renderer is not implemented yet!"); break;
        }
    }

    inline void run()
    {
        bool isAnyWindowOpen = true;  // In a future we'll support many windows at once

        while (isAnyWindowOpen)
        {
            isAnyWindowOpen = false;
            for (auto &window : { mMainWindow })
            {
                bool const close = window->isMarkedToClose();

                // RENDERER(update());
                RENDERER(draw());

                if (close)
                    window->destroy();  // WARNING : This should trigger something on 'SelectedRenderer' ??

                isAnyWindowOpen |= !close;
            }

            btm::Window::waitEvents();
        }
    }

    inline void cleanup()
    {
        RENDERER(cleanup());
        btm::Window::terminate();

        delete mRenderer;
        mRenderer = nullptr;
    }

    inline void reset() { *this = { mName }; }

    inline std::string name() const { return mName; }

    inline void markToClose() { mClose = true; }
    inline bool isMarkedToClose() const { return mClose; }

    inline glm::vec2 cursor() const { return mCursor; }
    inline void      cursor(glm::vec2 cursor) { mCursor = std::move(cursor); }

    inline Input::State key(Input::Key k) const { return mKeys.count(k) > 0 ? mKeys.at(k) : sDefaultState; }
    inline void         key(Input::Key k, Input::State s) { mKeys[k] = s; }

    inline Input::State mouse(Input::Mouse m) const { return mMouse.count(m) > 0 ? mMouse.at(m) : sDefaultState; }
    inline void         mouse(Input::Mouse m, Input::State s) { mMouse[m] = s; }

private:
    std::string mName = "";

    bool mInit  = false;
    bool mClose = false;

    glm::vec2                        mCursor = { 0, 0 };
    umap<Input::Key, Input::State>   mKeys   = {};
    umap<Input::Mouse, Input::State> mMouse  = {};

    Ref<btm::Window>   mMainWindow = nullptr;
    btm::BaseRenderer *mRenderer   = nullptr;
};

}  // namespace btm