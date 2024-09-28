#pragma once

#include "base.hpp"

#include "utils.hpp"
#include "transform.hpp"
#include "userInput.hpp"

namespace bm
{

class Camera
{
    static float constexpr sBound            = 1.5f;
    static float constexpr sFar              = 1'000.f;
    static float constexpr sNear             = 0.1f;
    static glm::vec3 constexpr sInitEye      = { 2.f, 4.f, 8.f };
    static float constexpr sInitZ            = sInitEye.z;
    static float constexpr sMouseSensitivity = 0.1f;

public:
    enum struct Mode
    {
        Fly,
        Orb,
        Ortho
    };

    Camera(std::string name, glm::vec3 eye = sInitEye, glm::vec3 lookAt = ZERO3, Mode mode = Mode::Fly)
      : mName(name)
      , mEye(eye)
      , mLookAt(lookAt)
      , mMode(mode)
      , mOrthoOffset(mEye.z)
    {
    }

    void update(float ar, glm::vec3 const &target = INF3)
    {
        if (target != INF3)
            move(mLookAt - target);

        // clang-format off
        auto const dir = Directions(front()) * speed();
        if (movU) { move(dir.U); }  // Move up
        if (movD) { move(dir.D); }  // Move down
        if (movF) { move(dir.F, dir.F * float(!useOrb()), -speed()); }  // Move forward
        if (movB) { move(dir.B, dir.B * float(!useOrb()), speed()); }   // Move backward
        if (movR) { move(dir.R); }  // Move right
        if (movL) { move(dir.L); }  // Move left

        float const xFlip = useOrb() ? -1.f : 1.f;
        float const yFlip = useOrb() ? 1.f : -1.f;
        if (rotU) { rotate(xFlip *  RIGHT * speed()); } // Look up
        if (rotD) { rotate(xFlip * -RIGHT * speed()); } // Look down
        if (rotR) { rotate(yFlip *  UP * speed()); }    // Look right
        if (rotL) { rotate(yFlip * -UP * speed()); }    // Look left
        // clang-format on

        mV = glm::lookAt(mEye, (useOrb() ? mLookAt : mEye + front()), UP);

        if (isOrtho())
        {
            float const mod = mOrthoOffset;
            float const w   = mod * ar;
            float const h   = mod;
            mP              = glm::ortho(w, -w, h, -h, -sFar * .5f, sFar * .5f);
        }
        else
            mP = glm::perspective(-glm::radians(mFOV), ar, sNear, sFar);
    }

    void onInputChange(UserInput const &ui)
    {
        mSpeedMod = ui.pressed(Key::LeftShift) ? 10.f : 1.f;

        // Toggle Orbital rotation
        if (ui.pressed(Key::O, true))
        {
            if (!useOrb())
            {
                mLookAt.z = 0.f;
            }
            mMode = useOrb() ? Mode::Fly : Mode::Orb;
        }

        // Toggle Orthographic projection
        if (ui.pressed(Key::Comma, true))
        {
            if (!isOrtho())
            {
                mOrthoOffset = mEye.z = std::copysignf(std::max(abs(mEye.z), sBound), mEye.z);
                mLookAt.z             = 0.f;
            }
            mMode = isOrtho() ? Mode::Fly : Mode::Ortho;
        }

        // Reset
        if (ui.pressed(Key::R))
            reset();

        // Movement
        movU = ui.pressed(Key::Q);
        movD = ui.pressed(Key::E);
        movF = ui.pressed(Key::W);
        movB = ui.pressed(Key::S);
        movR = ui.pressed(Key::D);
        movL = ui.pressed(Key::A);

        // Rotation
        rotU = ui.pressed(Key::I);
        rotD = ui.pressed(Key::K);
        rotR = ui.pressed(Key::L);
        rotL = ui.pressed(Key::J);

        // Zoom / Fov
        auto const fovMod  = ui.wheel().y * speed();
        auto const zoomMod = front() * fovMod;
        if (ui.pressed(Key::F))
            fov(fovMod * 100.f);
        else if (isOrtho())
            move(ZERO3, ZERO3, fovMod);
        else
            move(zoomMod, (useOrb() ? ZERO3 : zoomMod));

        // Rotation
        auto const rotMod = -ui.displ() * speed() * sMouseSensitivity;
        if (ui.clicked(Mouse::Left))
            rotate({ rotMod.y, rotMod.x, 0.f });
    }

    glm::mat4 V() const { return mV; }
    glm::mat4 P() const { return mP; }
    glm::mat4 VP() const { return mP * mV; }

    glm::vec3 front() { return mLookAt - mEye; }
    glm::vec3 right() { return Directions(front()).R; }
    glm::vec3 up() { return Directions(front()).U; }

    float fov() { return mFOV; }
    void  fov(float displ) { mFOV = glm::clamp((mFOV + displ * 0.5f), 4.f, 140.f); }

    float speed() { return mSpeed * mSpeedMod; }
    void  speed(float displ) { mSpeed += displ; }

    bool isOrb() { return mMode == Mode::Orb; }
    bool isFly() { return mMode == Mode::Fly; }
    bool isOrtho() { return mMode == Mode::Ortho; }
    bool useOrb() { return mMode == Mode::Orb || mMode == Mode::Ortho; }

    void reset() { *this = { mName }; }

private:
    void move(glm::vec3 const &displ) { move(displ, displ); }

    void move(glm::vec3 const &displEye, glm::vec3 const &displLookAt, float orthoInc = 0.f)
    {
        auto const auxE = mEye + displEye;
        auto const auxL = mLookAt + displLookAt;

        if (!useOrb() || (useOrb() && !math::fuzzyCmp(auxE, auxL, sBound)))
        {
            mEye = auxE;
        }

        if (!math::isAligned(auxL - mEye, UP) && !math::fuzzyCmp(mEye, auxL, sBound))
        {
            mLookAt = auxL;
        }

        mOrthoOffset = std::max(sBound, mOrthoOffset + orthoInc);
    }

    void rotate(glm::vec3 rotInc)
    {
        auto &satellite = !useOrb() ? mLookAt : mEye;
        auto &base      = !useOrb() ? mEye : mLookAt;

        satellite       = glm::rotate(satellite - base, rotInc.y, UP) + base;       // Rot Left <-> Right
        auto const next = glm::rotate(satellite - base, rotInc.x, right()) + base;  // Rot Up <-> Down

        if (!math::isAligned({ next - base }, UP, 0.01f))
        {
            satellite = next;
        }
    }

    // To Serialize
    std::string mName   = "";
    glm::vec3   mEye    = FRONT * sInitZ;
    glm::vec3   mLookAt = ZERO3;
    float       mFOV    = 75.f;
    float       mSpeed  = 0.01f;
    Mode        mMode   = Mode::Fly;

    // Dynamic
    bool movU = false, movD = false, movF = false, movB = false, movR = false, movL = false, rotR = false, rotL = false, rotU = false,
         rotD = false;

    glm::mat4 mV { 1.f };
    glm::mat4 mP { 1.f };

    float mSpeedMod     = 1.f;
    float mOrthoOffset  = sInitZ;
    bool  mUseWorldAxes = false;
};

class SmoothCamera
{
    Camera A;
    Camera B;
};

}  // namespace bm

// template<>
// struct fmt::formatter<bm::Camera>
// {
//     constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

//     template<typename FormatContext>
//     auto format(bm::Camera const &c, FormatContext &ctx) const -> decltype(ctx.out())
//     {
//         auto const boolStr = [](bool b) { return b ? "T" : "F"; };

//         auto const modeStr = [](bm::Camera::Mode m)
//         {
//             switch (m)
//             {
//                 case bm::Camera::Mode::Fly: return "Fly  ";
//                 case bm::Camera::Mode::Orb: return "Orb  ";
//                 case bm::Camera::Mode::Ortho: return "Ortho";
//                 default: return "";
//             }
//         };

//         return fmt::format_to(
//           ctx.out(),
//           "[CAMERA] '{}' -> E:{} | L:{} | F:{} | S:{} | W:{} | M:{} | O:{}",
//           mEye,
//           mLookAt,
//           mFOV,
//           speed(),
//           boolStr(mUseWorldAxes),
//           modeStr(mMode),
//           mOrthoOffset);
//     }
// };
