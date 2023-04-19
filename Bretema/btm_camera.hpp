#pragma once

#include "btm_base.hpp"
#include "btm_transform.hpp"

namespace btm
{

class Camera
{
public:
    inline void speed(float displ) { mSpeed += displ; }
    inline void fov(float displ) { mFOV = glm::clamp((mFOV + displ * 0.5f), 4.f, 140.f); }

    void update(float ar, glm::vec3 const &target = INF3)
    {
        if (target != INF3)
            move(mLookAt - target);

        mV = glm::lookAt(mEye, (mUseOrb ? mLookAt : mEye + front()), UP);

        if (mUseOrtho)
        {
            float const w = 10.f;
            float const h = 10.f;
            mP            = glm::ortho(w, -w, h, -h);
        }
        else
            mP = glm::perspective(-glm::radians(mFOV), ar, 0.1f, 100'000.f);
    }

    void onInputChange(UI::Info const &ui)
    {
        mSpeedMod = ui.pressed(UI::Key::LeftShift) ? 2.f : 1.f;

        if (ui.pressed(UI::Key::O, true))
        {
            if (!mUseOrb)
                mLookAt = ZERO3;
            mUseOrb = !mUseOrb;
        }

        if (ui.pressed(UI::Key::Comma, true))
            mUseOrtho = !mUseOrtho;

        // Reset
        if (ui.pressed(UI::Key::Num0))
        {
            *this = {};
        }

        // Movement
        // clang-format off
        auto const dir = Directions(front()) * speed();
        if (ui.pressed(UI::Key::Q)) { move(dir.U); }
        if (ui.pressed(UI::Key::E)) { move(dir.D); }
        if (ui.pressed(UI::Key::W)) { move(dir.F, dir.F * float(!mUseOrb)); }
        if (ui.pressed(UI::Key::S)) { move(dir.B, dir.B * float(!mUseOrb)); }
        if (ui.pressed(UI::Key::D)) { move(dir.R); }
        if (ui.pressed(UI::Key::A)) { move(dir.L); }
        // clang-format on

        // Zoom / Fov
        auto const fovMod  = ui.wheel().y * speed();
        auto const zoomMod = front() * fovMod;
        ui.pressed(UI::Key::LeftAlt) ? fov(fovMod) : move(zoomMod, (mUseOrb ? ZERO3 : zoomMod));

        // Rotation
        auto const rotMod = ui.displ() * speed() * 0.01f;
        if (ui.pressed(UI::Mouse::Left))
            rotate({ rotMod.y, -rotMod.x, 0.f });

        BTM_INFOF(
          "[CAMERA] -> E : {} | L : {} | F : {} | S : {} | Orb : {} | World : {} | Orth : {}",
          mEye,
          mLookAt,
          mFOV,
          mSpeed,
          mUseOrb,
          mUseWorldAxes,
          mUseOrtho);
    }

    inline glm::mat4 V() const { return mV; }
    inline glm::mat4 P() const { return mP; }
    inline glm::mat4 VP() const { return mP * mV; }

private:
    glm::vec3 front() { return mLookAt - mEye; }

    void move(glm::vec3 const &displ) { move(displ, displ); }

    void move(glm::vec3 const &displEye, glm::vec3 const &displLookAt)
    {
        auto const auxE = mEye + displEye;
        auto const auxL = mLookAt + displLookAt;

        if (!mUseOrb || (mUseOrb && !fuzzyCmp(auxE, auxL, 0.5f)))
            mEye = auxE;

        if (!isAligned(auxL - mEye, UP))
            mLookAt = auxL;
    }

    void rotate(glm::vec3 rotInc)
    {
        BTM_INFOF("to rot => {}", rotInc);
        auto const dir = Directions(front());

        glm::mat4 mx {}, my {};
        glm::rotate(mx, rotInc.y, UP);
        glm::rotate(my, rotInc.x, dir.R);

        auto &satellite = !mUseOrb ? mLookAt : mEye;
        auto &base      = !mUseOrb ? mEye : mLookAt;

        satellite = (mx * glm::vec4((satellite - base), 1.f)).xyz() + base;
        satellite = (my * glm::vec4((satellite - base), 1.f)).xyz() + base;

        // if (!isAligned({ next - base }, UP, 0.01f))
        // satellite = next;
    }

    // To Serialize
    glm::vec3 mEye    = FRONT * 6.f;
    glm::vec3 mLookAt = ZERO3;
    float     mFOV    = 75.f;
    float     mSpeed  = 1.f;

    // Dynamic
    struct
    {
        bool U, D, F, B, R, L;
        bool isOn() { return U || D || F || B || R || L; }
    } mMovement;

    glm::mat4 mV { 1.f };
    glm::mat4 mP { 1.f };

    float        mSpeedMod = 1.f;
    inline float speed() { return mSpeed * mSpeedMod; }

    bool mUseOrb       = false;
    bool mUseOrtho     = false;
    bool mUseWorldAxes = false;
};

}  // namespace btm