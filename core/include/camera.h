#pragma once

#include <matrix.h>

class Camera {
private:
    Vec3f m_pos;
    Vec3f m_up;
    Vec3f m_lookpoint;
    Mat4x4f m_viewMat;

private:
    void RefreshViewMatrix();

public:
    Camera(const Vec3f& pos, const Vec3f& up, const Vec3f& lookpoint):
        m_pos(pos), m_up(up), m_lookpoint(lookpoint) {
            RefreshViewMatrix();
        }

};