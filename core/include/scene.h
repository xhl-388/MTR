#pragma once

#include <camera.h>
#include <model.h>
#include <memory>

class Scene {
private:
    Camera m_camera;
    float m_projection_coeff;
    std::shared_ptr<Model> m_model;

    const int m_DEPTH,m_WIDTH,m_HEIGHT;
    Mat4x4f m_proMat,m_viewportMat;
    Vec2i m_viewport_offset;

private:
    void RefreshProjectionMatrix();

    void RefreshViewPortMatrix();


public:
    Scene(const Camera& camera, float projection_coeff, Model* model, int depth, int width, 
        int height, const Vec2i& viewport_offset)
        :m_camera(camera),m_projection_coeff(projection_coeff), m_model(model),
        m_DEPTH(depth), m_WIDTH(width), m_HEIGHT(height), m_viewport_offset(viewport_offset){
            RefreshProjectionMatrix();
            RefreshViewPortMatrix();
        }

    void Render();
};