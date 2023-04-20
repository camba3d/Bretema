#pragma once

#include "btm_base.hpp"
#include "btm_transform.hpp"

namespace btm
{

class Camera
{
public:
    void update(float ar, glm::vec3 const &target = INF3)
    {
        if (target != INF3)
            move(mLookAt - target);

        // clang-format off
        auto const dir = Directions(front()) * speed();

        if (movU) { move(dir.U); }  // Move up
        if (movD) { move(dir.D); }  // Move down
        if (movF) { move(dir.F, dir.F * float(!mUseOrb), -speed()); }  // Move forward
        if (movB) { move(dir.B, dir.B * float(!mUseOrb), speed()); }  // Move backward
        if (movR) { move(dir.R); }  // Move right
        if (movL) { move(dir.L); }  // Move left

        if (rotU) { rotate( RIGHT * speed()); } // Look up
        if (rotD) { rotate(-RIGHT * speed()); } // Look down
        if (rotR) { rotate(-UP    * speed()); } // Look right
        if (rotL) { rotate( UP    * speed()); } // Look left
        // clang-format on

        mV = glm::lookAt(mEye, (mUseOrb ? mLookAt : mEye + front()), UP);

        if (mUseOrtho)
        {
            float const mod = mOrthoOffset;
            float const w   = ar * mod;
            float const h   = 1.f * mod;
            mP              = glm::ortho(w, -w, h, -h, 0.1f, 1'000.f);
        }
        else
            mP = glm::perspective(-glm::radians(mFOV), ar, 0.1f, 100'000.f);
    }

    void onInputChange(UI::Info const &ui)
    {
        mSpeedMod = ui.pressed(UI::Key::LeftShift) ? 10.f : 1.f;

        if (ui.pressed(UI::Key::O, true))
        {
            if (!mUseOrb)
                mLookAt = ZERO3;
            mUseOrb = !mUseOrb;
        }

        if (ui.pressed(UI::Key::Comma, true))
        {
            if (!mUseOrtho)
            {
                mOrthoOffset = mEye.z - 1.25f;
                mEye.z       = std::copysignf(mLookAt.z, 6.f);
                mLookAt.z    = 0;  // wip : what to do when eye and lookat are negative
            }
            else
            {
                float const diff = (mOrthoOffset - mEye.z) + 1.25f;
                mEye.z += diff;
                mLookAt.z += diff;
            }

            mUseOrtho = !mUseOrtho;
        }

        // Reset
        if (ui.pressed(UI::Key::Num0))
        {
            *this = {};
        }

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
        else if (mUseOrtho)
            move(ZERO3, ZERO3, fovMod);
        else
            move(zoomMod, (mUseOrb ? ZERO3 : zoomMod));

        // Rotation
        auto const rotMod = -ui.displ() * speed();
        if (ui.pressed(UI::Mouse::Left))
            rotate({ rotMod.y, rotMod.x, 0.f });

        BTM_INFOF(
          "[CAMERA] -> E : {} | L : {} | F : {} | S : {} | Orb : {} | World : {} | Ortho : {} / {}",
          mEye,
          mLookAt,
          mFOV,
          speed(),
          mUseOrb,
          mUseWorldAxes,
          mUseOrtho,
          mOrthoOffset);
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

private:
    void move(glm::vec3 const &displ) { move(displ, displ); }

    void move(glm::vec3 const &displEye, glm::vec3 const &displLookAt, float orthoInc = 0.f)
    {
        auto const auxE = mEye + displEye * (mUseOrtho ? XY3 : XYZ3);
        auto const auxL = mLookAt + displLookAt * (mUseOrtho ? XY3 : XYZ3);

        if (!mUseOrb || (mUseOrb && !fuzzyCmp(auxE, auxL, 0.5f)))
            mEye = auxE;

        if (!isAligned(auxL - mEye, UP) && !fuzzyCmp(mEye, auxL, 0.5f))
            mLookAt = auxL;

        mOrthoOffset = std::max(0.5f, mOrthoOffset + orthoInc);
    }

    void rotate(glm::vec3 rotInc)
    {
        auto &satellite = !mUseOrb ? mLookAt : mEye;
        auto &base      = !mUseOrb ? mEye : mLookAt;

        satellite       = glm::rotate(satellite - base, rotInc.y, UP) + base;       // Rot Left <-> Right
        auto const next = glm::rotate(satellite - base, rotInc.x, right()) + base;  // Rot Up <-> Down

        if (!isAligned({ next - base }, UP, 0.1f))
            satellite = next;
    }

    // Dynamic
    bool movU, movD, movF, movB, movR, movL, rotR, rotL, rotU, rotD;

    glm::mat4 mV { 1.f };
    glm::mat4 mP { 1.f };

    float mSpeedMod    = 1.f;
    float mOrthoOffset = 6.f;

    bool mUseOrb       = false;
    bool mUseOrtho     = false;
    bool mUseWorldAxes = false;

    // To Serialize
    glm::vec3 mEye    = FRONT * mOrthoOffset;
    glm::vec3 mLookAt = ZERO3;
    float     mFOV    = 75.f;
    float     mSpeed  = 0.01f;
};

}  // namespace btm