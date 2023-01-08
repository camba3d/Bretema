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

//===========================
//= TYPEDEFS
//===========================

using UUID = std::string;

//===========================
//= CONTANTS
//===========================

glm::vec3 constexpr RIGHT = { 1, 0, 0 };
glm::vec3 constexpr UP    = { 0, 1, 0 };
glm::vec3 constexpr FRONT = { 0, 0, 1 };

//===========================
//= ENUMS
//===========================

enum struct Cull
{
    NONE,
    CW,
    CCW,
};

enum struct Compare
{
    NONE,
    LESS,
    LESS_EQ,
    GREAT,
    GREAT_EQ
};
using Depth = Compare;

enum struct Samples
{
    _1,
    _2,
    _4,
    _8,
    _16,
    _32,
    _64,
};

//===========================
//= HELPER STRUCTS
//===========================

struct Area2D
{
    glm::vec2 off = {};  // Rect's init-point
    glm::vec2 ext = {};  // Rect's end-point
};

struct Area3D
{
    glm::vec3 off = {};  // Cube's init-point
    glm::vec3 ext = {};  // Cube's end-point
};

//===========================
//= BASE RENDERER
//===========================

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