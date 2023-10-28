#include "btm_app.hpp"

namespace btm
{

// clang-format off
#define RENDERER(call) do { BTM_ASSERT(mRenderer); (mRenderer)->call; } while (0);
// clang-format on

App::App(std::string name, RenderAPI renderAPI) : mName(std::move(name)), mRenderAPI(renderAPI)
{
    BTM_ABORTF_IF(
      mMainWindow || mRenderer,
      "Initialization fiasco... Window({}) or Renderer({}) already exists!",
      BTM_STR_PTR(mMainWindow),
      BTM_STR_PTR(mRenderer));

    // Init window
    mMainWindow = sNew<Window>(1920, 1080, "Default Window", this);

    // Init renderer
    switch (mRenderAPI)
    {
        case Vulkan: mRenderer = new vk::Renderer(mMainWindow); break;
        default: BTM_ABORT("Selected renderer is not implemented yet!"); break;
    }
}

void App::run()
{
    bool isAnyWindowOpen = true;  // In a future we'll support many windows at once

    while (isAnyWindowOpen)
    {
        isAnyWindowOpen = false;
        for (auto &window : { mMainWindow })
        {
            std::string et = mETimer.elapsedStr();
            window->titleInfo(et);
            mETimer.reset();

            bool const close = window->isMarkedToClose();

            // RENDERER(update());
            for (auto &camera : mCameras)
            {
                camera.update(1.77777f, INF3);
            }

            auto const &mainCamera = mCameras.at(0);
            RENDERER(draw(mainCamera));

            if (close)
                window->destroy();  // WARNING : This should trigger something on 'SelectedRenderer' ??

            isAnyWindowOpen |= !close;
        }

        // btm::Window::waitEvents();
        btm::Window::pollEvents();
    }
}

void App::cleanup()
{
    RENDERER(cleanup());
    btm::Window::terminate();

    delete mRenderer;
    mRenderer = nullptr;
}

void App::reset()
{
    *this = { mName, mRenderAPI };
}

std::string App::name() const
{
    return mName;
}

void App::markToClose()
{
    mClose = true;
}
bool App::isMarkedToClose() const
{
    return mClose;
}

}  // namespace btm