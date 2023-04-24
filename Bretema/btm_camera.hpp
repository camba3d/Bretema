#pragma once

#include "btm_base.hpp"
#include "btm_transform.hpp"

namespace btm
{

class Camera
{
    static float constexpr sBound            = 1.5f;
    static float constexpr sFar              = 1'000.f;
    static float constexpr sNear             = 0.1f;
    static float constexpr sInitZ            = 6.f;
    static float constexpr sMouseSensitivity = 0.1f;

public:
    enum struct Mode
    {
        Fly,
        Orb,
        Ortho
    };

    Camera(std::string name, glm::vec3 eye = FRONT * sInitZ, glm::vec3 lookAt = ZERO3, Mode mode = Mode::Fly) :
      mName(name),
      mEye(eye),
      mLookAt(lookAt),
      mMode(mode),
      mOrthoOffset(mEye.z)
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

    void onInputChange(UI::Info const &ui)
    {
        mSpeedMod = ui.pressed(UI::Key::LeftShift) ? 10.f : 1.f;

        // Toggle Orbital rotation
        if (ui.pressed(UI::Key::O, true))
        {
            if (!useOrb())
            {
                mLookAt.z = 0.f;
            }
            mMode = useOrb() ? Mode::Fly : Mode::Orb;
        }

        // Toggle Orthographic projection
        if (ui.pressed(UI::Key::Comma, true))
        {
            if (!isOrtho())
            {
                mOrthoOffset = mEye.z = std::copysignf(std::max(abs(mEye.z), sBound), mEye.z);
                mLookAt.z             = 0.f;
            }
            mMode = isOrtho() ? Mode::Fly : Mode::Ortho;
        }

        // Reset
        if (ui.pressed(UI::Key::R))
            reset();

        // Movement
        movU = ui.pressed(UI::Key::Q);
        movD = ui.pressed(UI::Key::E);
        movF = ui.pressed(UI::Key::W);
        movB = ui.pressed(UI::Key::S);
        movR = ui.pressed(UI::Key::D);
        movL = ui.pressed(UI::Key::A);

        // Rotation
        rotU = ui.pressed(UI::Key::I);
        rotD = ui.pressed(UI::Key::K);
        rotR = ui.pressed(UI::Key::L);
        rotL = ui.pressed(UI::Key::J);

        // Zoom / Fov
        auto const fovMod  = ui.wheel().y * speed();
        auto const zoomMod = front() * fovMod;
        if (ui.pressed(UI::Key::LeftAlt))
            fov(fovMod);
        else if (isOrtho())
            move(ZERO3, ZERO3, fovMod);
        else
            move(zoomMod, (useOrb() ? ZERO3 : zoomMod));

        // Rotation
        auto const rotMod = -ui.displ() * speed() * sMouseSensitivity;
        if (ui.pressed(UI::Mouse::Left))
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

        if (!useOrb() || (useOrb() && !fuzzyCmp(auxE, auxL, sBound)))
            mEye = auxE;

        if (!isAligned(auxL - mEye, UP) && !fuzzyCmp(mEye, auxL, sBound))
            mLookAt = auxL;

        mOrthoOffset = std::max(sBound, mOrthoOffset + orthoInc);
    }

    void rotate(glm::vec3 rotInc)
    {
        auto &satellite = !useOrb() ? mLookAt : mEye;
        auto &base      = !useOrb() ? mEye : mLookAt;

        satellite       = glm::rotate(satellite - base, rotInc.y, UP) + base;       // Rot Left <-> Right
        auto const next = glm::rotate(satellite - base, rotInc.x, right()) + base;  // Rot Up <-> Down

        if (!isAligned({ next - base }, UP, 0.1f))
            satellite = next;
    }

    // To Serialize
    std::string mName   = "";
    glm::vec3   mEye    = FRONT * sInitZ;
    glm::vec3   mLookAt = ZERO3;
    float       mFOV    = 75.f;
    float       mSpeed  = 0.01f;
    Mode        mMode   = Mode::Fly;

    // Dynamic
    bool movU, movD, movF, movB, movR, movL, rotR, rotL, rotU, rotD;

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

}  // namespace btm

   // template<>
// struct fmt::formatter<btm::Camera>
// {
//     constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

//     template<typename FormatContext>
//     auto format(btm::Camera const &c, FormatContext &ctx) const -> decltype(ctx.out())
//     {
//         auto const boolStr = [](bool b) { return b ? "T" : "F"; };

//         auto const modeStr = [](btm::Camera::Mode m)
//         {
//             switch (m)
//             {
//                 case btm::Camera::Mode::Fly: return "Fly  ";
//                 case btm::Camera::Mode::Orb: return "Orb  ";
//                 case btm::Camera::Mode::Ortho: return "Ortho";
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