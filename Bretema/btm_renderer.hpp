#pragma once

/* NOTE:
 *
 *  Include the <vk/dx/gl/mt/wg>-Renderer files before
 *  the BaseRenderer like :
 *
 *    #include "vk_renderer.hpp"
 *    #include "../btm_renderer.hpp"
 *
 */

#include "btm_base.hpp"
#include "btm_window.hpp"

namespace btm
{

class BaseRenderer
{
public:
    inline static constexpr int32_t sInFlight = 3;

    BaseRenderer(Ref<btm::Window> window);
    virtual ~BaseRenderer() = default;

    virtual void update() { BTM_ASSERT_X(0, "Cast-back to non-base renderer!"); }
    virtual void draw() { BTM_ASSERT_X(0, "Cast-back to non-base renderer!"); }
    virtual void cleanup() { BTM_ASSERT_X(0, "Cast-back to non-base renderer!"); }

    inline bool isInitialized() { return mInit; }

protected:
    inline void markAsInit() { mInit = true; }

    int32_t mFrameNumber = 0;

    bool      mInit         = false;
    void     *mWindowHandle = nullptr;
    glm::vec2 mViewportSize = { 1280, 720 };
};

}  // namespace btm