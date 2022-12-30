#include "btm_renderer.hpp"

namespace btm
{

BaseRenderer::BaseRenderer(Ref<btm::Window> window)
{
    mWindowHandle = window->handle();
    mViewportSize = window->size();

    BTM_ASSERT_X(mWindowHandle, "Invalid window handle");
    BTM_ASSERT_X(mViewportSize.x > 0 && mViewportSize.y > 0, "Invalid viewport size");
}

}  // namespace btm