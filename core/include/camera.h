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

    Mat4x4f GetViewMatrix() const {
        return m_viewMat;
    }
    void LookAt(const Vec3f& pos, const Vec3f& up, const Vec3f& lookpoint);

    void SetPosition(Vec3f newPos) {
        m_pos = newPos;
        RefreshViewMatrix();
    }

    Vec3f GetPosition() const {
        return m_pos;
    }

    Vec3f GetLookPoint() const {
        return m_lookpoint;
    }
};