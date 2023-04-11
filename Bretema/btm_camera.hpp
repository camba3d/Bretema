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
        if (mPivot.rotZ() != 0.f)
            mPivot.rot().z = 0.f;

        // // Avoid camera flip at 90º
        // if (mPivot.rotX() >= 90.f and mPivot.rotX() < 180.f)
        //     mPivot.rot().x = 89.9f;

        // // Avoid camera flip at 270º
        // if (mPivot.rotX() >= 180.f and mPivot.rotX() < 270.f)
        //     mPivot.rot().x = 270.f;

        auto const [F, R, U] = mPivot.getVectors();
        auto const [B, L, D] = std::make_tuple(-F, -R, -U);

        // Movement
        //----------
        float const speed = realSpeed();

        if (mIsOrbital)
        {
            mOffset.z -= speed * float(mMovement.F);
            mOffset.z += speed * float(mMovement.B);
            mOffset.x += speed * float(mMovement.R);
            mOffset.x -= speed * float(mMovement.L);
            mOffset.y += speed * float(mMovement.D);
            mOffset.y -= speed * float(mMovement.U);
        }
        else  // Fly
        {
            mPivot.pos() += (F * speed) * float(mMovement.F);
            mPivot.pos() += (B * speed) * float(mMovement.B);
            mPivot.pos() += (R * speed) * float(mMovement.R);
            mPivot.pos() += (L * speed) * float(mMovement.L);
            mPivot.pos() += (U * speed) * float(mMovement.U);
            mPivot.pos() += (D * speed) * float(mMovement.D);
        }

        // Matrices
        //----------
        mEye = mPivot.pos() + (mOffset.x * R) + (mOffset.y * U) - (mOffset.z * F);

        mV = glm::lookAt(mEye, mEye + F, UP);
        mP = glm::perspective(-glm::radians(mFOV), ar, 0.1f, 100'000.f);
    }

    void onInputChange(UI::Info const &ui)
    {
        mSpeedMod = ui.pressed(UI::Key::LeftShift) ? 2.f : 1.f;

        if (ui.pressed(UI::Key::O, true))
            mIsOrbital = !mIsOrbital;

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
        auto const displ = -ui.displ() * speed;
        if (ui.pressed(UI::Mouse::Left))
            mPivot.rot() += glm::vec3(displ.y, displ.x, 0.f);

        BTM_INFOF("[CAMERA] -> e:{} | o:{} | f:{} | s:{}", mEye, mOffset, mFOV, speed);
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
    glm::vec3 mOffset = FRONT * 6.f;

    Transform mPivot = {};

    float        mSpeed    = 1.f;
    float        mSpeedMod = 1.f;
    inline float realSpeed() { return mSpeed * mSpeedMod; }

    bool mIsOrbital = false;
};

}  // namespace btm