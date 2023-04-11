#pragma once

#include "btm_base.hpp"

namespace btm
{

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

        glm::mat4 S = glm::scale(glm::mat4 { 1.f }, mScl * -FRONT);

        return T * R * S;
    }

    std::tuple<glm::vec3, glm::vec3, glm::vec3> getVectors() const
    {
        glm::vec3 F = normalize(matrix()[2]).xyz();
        glm::vec3 R = normalize(cross(F, UP));
        glm::vec3 U = normalize(cross(F, R));

        return { F, R, U };
    }

    void reset() { *this = {}; }

    // TRANSLATION
    inline float posX() { return mPos.x; }
    inline float posY() { return mPos.y; }
    inline float posZ() { return mPos.z; }

    inline glm::vec3  pos() const { return mPos; }
    inline glm::vec3 &pos() { return mPos; }

    // SCALE
    inline float sclX() { return mScl.x; }
    inline float sclY() { return mScl.y; }
    inline float sclZ() { return mScl.z; }

    inline glm::vec3  scl() const { return mScl; }
    inline glm::vec3 &scl() { return mScl; }

    // ROTATION
    inline float rotX() const { return safeRot().x; }
    inline float rotY() const { return safeRot().y; }
    inline float rotZ() const { return safeRot().z; }

    inline glm::vec3  rot() const { return safeRot(); }
    inline glm::vec3 &rot() { return safeRot(); }

private:
    glm::vec3         mPos { 0.f };
    mutable glm::vec3 mRot { 0.f };
    glm::vec3         mScl { 1.f };

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