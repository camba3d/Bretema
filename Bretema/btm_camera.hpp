#pragma once

#include "btm_base.hpp"
#include "btm_transform.hpp"

namespace btm
{

class Camera
{
public:
    void setZoom(float displ) { mOffset.z += displ; }

    void setSpeed(float displ) { mSpeed += displ; }

    void setFOV(float displ)
    {
        mFOV += glm::radians(displ);
        mFOV = glm::clamp(mFOV, 4.0f, 120.0f);
    }

    void update(float ar, bool isOrbital, glm::vec3 const &target = INF3)
    {
        if (target != INF3)
            mPivot.pos() = target;

        // Avoid any ROLL
        if (mPivot.rotZ() != 0.f)
            mPivot.rot().z = 0.f;

        // Avoid camera flip at 90º
        if (mPivot.rotX() >= 90.f and mPivot.rotX() < 180.f)
            mPivot.rot().x = 89.9f;

        // Avoid camera flip at 270º
        if (mPivot.rotX() >= 180.f and mPivot.rotX() < 270.f)
            mPivot.rot().x = 270.f;

        auto const [F, R, U] = mPivot.getVectors();

        // Movement
        //----------
        if (isOrbital)
        {
            if (mMovement.F)
                mOffset.z -= mSpeed;
            if (mMovement.B)
                mOffset.z += mSpeed;
            if (mMovement.R)
                mOffset.x += mSpeed;
            if (mMovement.L)
                mOffset.x -= mSpeed;
            if (mMovement.U)
                mOffset.y -= mSpeed;
            if (mMovement.D)
                mOffset.y += mSpeed;
        }
        else
        {  // * FPS
            if (mMovement.F)
                mPivot.pos() += F * mSpeed;
            if (mMovement.B)
                mPivot.pos() += -F * mSpeed;
            if (mMovement.R)
                mPivot.pos() += R * mSpeed;
            if (mMovement.L)
                mPivot.pos() += -R * mSpeed;
            if (mMovement.U)
                mPivot.pos() += -U * mSpeed;
            if (mMovement.D)
                mPivot.pos() += U * mSpeed;
        }

        // Matrices
        //----------
        glm::vec3 const eye = mPivot.pos() + (mOffset.x * R) + (mOffset.y * U) - (mOffset.z * F);
        mV                  = glm::lookAt(eye, eye + F, UP);
        mP                  = glm::perspective(mFOV, ar, 0.1f, 100'000.f);
    }

    void onInputUpdate(
      glm::vec2 const                        &displ,
      glm::vec2 const                        &currPos,
      umap<Input::Mouse, Input::State> const &mouse,
      umap<Input::Key, Input::State>          keys)
    {
        // Camera movement
        //   if (keys.at(Input::Key::Q) ==Input::State::Press) { movement.U = true; } // Up
        //   if (!aIO.keyHold(KeyCode::Q)) { movement.U = false; }

        //   if (aIO.keyHold(KeyCode::E)) { movement.D = true; } // Down
        //   if (!aIO.keyHold(KeyCode::E)) { movement.D = false; }
        //   if (aIO.keyHold(KeyCode::W)) { movement.F = true; } // Front
        //   if (!aIO.keyHold(KeyCode::W)) { movement.F = false; }
        //   if (aIO.keyHold(KeyCode::S)) { movement.B = true; } // Back
        //   if (!aIO.keyHold(KeyCode::S)) { movement.B = false; }
        //   if (aIO.keyHold(KeyCode::D)) { movement.R = true; } // Right
        //   if (!aIO.keyHold(KeyCode::D)) { movement.R = false; }
        //   if (aIO.keyHold(KeyCode::A)) { movement.L = true; } // Left
        //   if (!aIO.keyHold(KeyCode::A)) { movement.L = false; }
        //   if (aIO.keyHold(KeyCode::Num0)) { pivot.reset(); } // Reset

        // Scroll for Fov / Shift + Scroll for Zoom
        // (aIO.keyHold(KeyCode::LeftShift)) ? setZoom(aIO.scrollV() * 10.f) : setFOV(aIO.scrollV())
        // (aIO.keyHold(KeyCode::LeftShift)) ? setZoom(aIO.scrollV()) : setFOV(aIO.scrollV());

        // Cmd/Ctrl: to rotate camera
        // pivot.modRot(glm::vec3{aIO.axisV(), aIO.axisH(), 0.f} * 0.25f);
    }

    glm::mat4 V() const { return mV; }
    glm::mat4 P() const { return mP; }
    glm::mat4 VP() const { return mP * mV; }

private:
    struct
    {
        bool U, D, F, B, R, L;
    } mMovement;

    glm::mat4 mV { 1.f };
    glm::mat4 mP { 1.f };

    float     mFOV { 0.4f };
    glm::vec3 mOffset { 0.f, 0.f, 1.f };

    Transform mPivot = {};

    float mSpeed = 1.f;
};

}  // namespace btm