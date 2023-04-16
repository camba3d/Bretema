#pragma once

#include "btm_base.hpp"

namespace btm
{

struct Directions
{
    Directions(glm::vec3 const &front)
    {
        F = glm::normalize(front);
        R = glm::normalize(glm::cross(F, UP));
        U = glm::normalize(glm::cross(F, R));
        B = -F;
        L = -R;
        D = -U;
    }

    Directions(glm::mat4 const &m) : Directions(m[2].xyz()) {}

    glm::vec3 F = {};
    glm::vec3 R = {};
    glm::vec3 U = {};
    glm::vec3 B = {};
    glm::vec3 L = {};
    glm::vec3 D = {};
};

class Transform
{
public:
    glm::mat4 matrix() const
    {
        glm::mat4 T = glm::translate(glm::mat4 { 1.f }, mPos);

        glm::mat4 R { 1.f };
        R = glm::rotate(R, glm::radians(safeRot().z), FRONT);
        R = glm::rotate(R, glm::radians(safeRot().y), UP);
        R = glm::rotate(R, glm::radians(safeRot().x), RIGHT);

        glm::mat4 S = glm::scale(glm::mat4 { 1.f }, mScl);

        return T * R * S;
    }

    Directions directions() const { return Directions(matrix()); }

    void setFront(glm::vec3 const &front)
    {
        auto const dir = directions();
        auto const A   = glm::normalize(dir.F);
        auto const B   = glm::normalize(front);
        mRot           = glm::degrees(glm::eulerAngles(glm::rotation(A, B)));
        BTM_INFOF("rot... {} | {} | {}", A, B, mRot);
    }

    void reset() { *this = {}; }

    // TRANSLATION
    inline glm::vec3  pos() const { return mPos; }
    inline glm::vec3 &pos() { return mPos; }

    // SCALE
    inline glm::vec3  scl() const { return mScl; }
    inline glm::vec3 &scl() { return mScl; }

    // ROTATION
    inline glm::vec3  rot() const { return safeRot(); }
    inline glm::vec3 &rot() { return safeRot(); }

    // // PIVOT
    // inline glm::vec3  pivot() const { return mPivot; }
    // inline glm::vec3 &pivot() { return mPivot; }

private:
    glm::vec3         mPos { 0.f };
    mutable glm::vec3 mRot { 0.f };
    glm::vec3         mScl { 1.f };

    // glm::vec3 mPivot = INF3;

    glm::vec3 &safeRot() const
    {
        if (mRot.x < 0.f || mRot.x > 360.f)
            mRot.x = clampRot(mRot.x);
        if (mRot.y < 0.f || mRot.y > 360.f)
            mRot.y = clampRot(mRot.y);
        if (mRot.z < 0.f || mRot.z > 360.f)
            mRot.z = clampRot(mRot.z);

        return mRot;
    }
};

}  // namespace btm