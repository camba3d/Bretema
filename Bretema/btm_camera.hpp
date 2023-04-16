#pragma once

#include "btm_base.hpp"
#include "btm_transform.hpp"

namespace btm
{

class Camera
{
public:
    inline void setZoom(float displ) { mOffset.z += displ; }

    inline void setSpeed(float displ) { mSpeed += displ; }

    inline void setFOV(float displ) { mFOV = glm::clamp((mFOV + displ * 0.5f), 4.f, 140.f); }

    void update(float ar, glm::vec3 const &target = INF3)
    {
        if (target != INF3)
            mPivot.pos() = target;
        // Avoid any ROLL
        if (mPivot.rot().z != 0.f)
            mPivot.rot().z = 0.f;
        // Avoid camera flip at 90º
        if (mPivot.rot().x >= 90.f and mPivot.rot().x < 180.f)
            mPivot.rot().x = 90.f - 0.2f;
        // Avoid camera flip at 270º
        if (mPivot.rot().x >= 180.f and mPivot.rot().x < 270.f)
            mPivot.rot().x = 270.f + 0.2f;

        auto const dir = mPivot.directions();

        // Movement
        //----------
        float const speed = realSpeed();

        if (mIsOrbital)
        {
            mOffset.z += speed * float(mMovement.F);
            mOffset.z -= speed * float(mMovement.B);
            mOffset.x -= speed * float(mMovement.R);
            mOffset.x += speed * float(mMovement.L);
            mOffset.y += speed * float(mMovement.U);
            mOffset.y -= speed * float(mMovement.D);
        }
        else  // Fly
        {
            mPivot.pos() += (dir.F * speed) * float(mMovement.F);
            mPivot.pos() += (dir.B * speed) * float(mMovement.B);
            mPivot.pos() += (dir.R * speed) * float(mMovement.R);
            mPivot.pos() += (dir.L * speed) * float(mMovement.L);
            mPivot.pos() += (dir.D * speed) * float(mMovement.U);
            mPivot.pos() += (dir.U * speed) * float(mMovement.D);
        }

        // Matrices
        //----------
        mEye = mPivot.pos() + (mOffset.x * dir.R) + (mOffset.y * dir.U) + (mOffset.z * dir.F);

        mV = glm::lookAt(mEye, mEye + dir.F, UP);
        mP = glm::perspective(-glm::radians(mFOV), ar, 0.1f, 100'000.f);
    }

    void onInputChange(UI::Info const &ui)
    {
        mSpeedMod = ui.pressed(UI::Key::LeftShift) ? 2.f : 1.f;

        if (ui.pressed(UI::Key::O, true))
        {
            if (mIsOrbital)  // Orb to Fly
            {
                mPivot.pos() = mEye;
                mOffset      = ZERO3;
            }
            else  // Fly to Orb
            {
                mOffset      = mEye;
                mPivot.pos() = ZERO3;

                // mPivot.setFront(-mOffset);
            }
            mIsOrbital = !mIsOrbital;
        }

        float const speed = realSpeed();

        // Reset
        if (ui.pressed(UI::Key::Num0))
            mPivot.reset();

        // Movement
        mMovement.U = ui.pressed(UI::Key::Q);
        mMovement.D = ui.pressed(UI::Key::E);
        mMovement.F = ui.pressed(UI::Key::W);
        mMovement.B = ui.pressed(UI::Key::S);
        mMovement.R = ui.pressed(UI::Key::D);
        mMovement.L = ui.pressed(UI::Key::A);

        // Zoom / Fov
        float const wheelY = ui.wheel().y * speed;
        if (ui.pressed(UI::Key::LeftAlt))
            setFOV(wheelY);
        else
            setZoom(wheelY);

        // Rotation
        auto const displ = ui.displ() * speed * 0.1f;
        if (ui.pressed(UI::Mouse::Left))
            mPivot.rot() += glm::vec3(displ.y, -displ.x, 0.f);

        BTM_INFOF(
          "[CAMERA] -> e:{} | r:{} | p:{} | o:{} | f:{} | s:{} | orb:{}",
          mEye,
          mPivot.rot(),
          mPivot.pos(),
          mOffset,
          mFOV,
          speed,
          mIsOrbital);
    }

    inline glm::mat4 V() const { return mV; }
    inline glm::mat4 P() const { return mP; }
    inline glm::mat4 VP() const { return mP * mV; }

private:
    struct
    {
        bool U, D, F, B, R, L;
    } mMovement;

    glm::mat4 mV { 1.f };
    glm::mat4 mP { 1.f };

    float     mFOV    = 75.f;
    glm::vec3 mEye    = ZERO3;
    glm::vec3 mOffset = ZERO3;

    Transform mPivot = []()
    {
        Transform t;
        t.pos().z = -6;
        return t;
    }();

    float        mSpeed    = 1.f;
    float        mSpeedMod = 1.f;
    inline float realSpeed() { return mSpeed * mSpeedMod; }

    bool mIsOrbital = false;

    ////////
    // implment rotation pivot on transform ??
    ////////
};

}  // namespace btm