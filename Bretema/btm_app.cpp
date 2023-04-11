#include "btm_app.hpp"

namespace btm
{

// clang-format off
#define RENDERER(call) do { BTM_ASSERT(mRenderer); (mRenderer)->call; } while (0);

App::App(std::string name, RenderAPI renderAPI) : mName(std::move(name)), mRenderAPI(renderAPI)
{
    BTM_ABORTF_IF(
      mMainWindow || mRenderer,
      "Initialization fiasco... Window({}) or Renderer({}) already exists!",
      BTM_PTRSTR(mMainWindow),
      BTM_PTRSTR(mRenderer));

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
            bool const close = window->isMarkedToClose();

            // RENDERER(update());
            RENDERER(draw());

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

glm::vec2 App::cursor() const
{
    return mCursor;
}
void App::cursor(glm::vec2 cursor)
{
    glm::vec2 const displ = cursor - mCursor;
    mCursor = std::move(cursor);

    for (auto & camera : mCameras)
        camera.onInputUpdate(displ, mCursor, mMouse, mKeys);
}

Input::State App::key(Input::Key k) const
{
    return mKeys.count(k) > 0 ? mKeys.at(k) : sDefaultState;
}
void App::key(Input::Key k, Input::State s)
{
    mKeys[k] = s;

    for (auto & camera : mCameras)
        camera.onInputUpdate(displ, mCursor, mMouse, mKeys);
}

Input::State App::mouse(Input::Mouse m) const
{
    return mMouse.count(m) > 0 ? mMouse.at(m) : sDefaultState;
}
void App::mouse(Input::Mouse m, Input::State s)
{
    mMouse[m] = s;
}

}  // namespace btm