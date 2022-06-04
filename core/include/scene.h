#pragma once

#include <camera.h>
#include <model.h>
#include <memory>
#include <climits>
#include <light.h>
#include <utils.h>
#include <omp.h>

struct IShader;

class Scene {
    friend class ZShader;
    friend class GouraudShader;
    friend class DepthShader;
private:
    Camera m_camera;
    float m_projection_coeff;
    std::shared_ptr<Model> m_model;

    const int m_DEPTH,m_WIDTH,m_HEIGHT;
    Mat4x4f m_proMat,m_viewportMat;
    Vec2i m_viewport_offset;

    float *m_zbuffer, *m_shadowbuffer;

    Light m_light;

    TGAImage m_image;
	TGAImage m_depthImg;
	TGAImage m_frame;

    const char* m_outputDir;
private:
    void RefreshProjectionMatrix();

    void RefreshViewPortMatrix();

    Mat4x4f GetViewMatrix() const {
        return m_camera.GetViewMatrix();
    }

    void SetProjectionCoeff(float newCoeff) {
        m_projection_coeff = newCoeff;
        RefreshProjectionMatrix();
    }
    float max_elevation_angle(float *buffer, Vec2f p, Vec2f dir);

    static Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P);

    void triangle(Mat<float,4,3>& clipc, TGAImage &image, IShader& shader, float* zbuffer);

public:
    Scene(const Camera& camera, float projection_coeff, Model* model, int depth, int width, 
        int height, const Vec2i& viewport_offset, const Light& light, const char* outputDir)
        :m_camera(camera),m_projection_coeff(projection_coeff), m_model(model),
        m_DEPTH(depth), m_WIDTH(width), m_HEIGHT(height), m_viewport_offset(viewport_offset),
        m_zbuffer(new float[m_WIDTH * m_HEIGHT]), m_shadowbuffer(new float[m_WIDTH * m_HEIGHT]),
        m_light(light), m_image(width,height,TGAImage::RGB), m_depthImg(width,height,TGAImage::GRAYSCALE),
        m_frame(width,height,TGAImage::RGB), m_outputDir(outputDir){
            RefreshProjectionMatrix();
            RefreshViewPortMatrix();
	        std::fill(m_zbuffer, m_zbuffer + width*height, -std::numeric_limits<float>::max());
	        std::fill(m_shadowbuffer, m_shadowbuffer + width*height,
                -std::numeric_limits<float>::max());
        }

    void Render();

    ~Scene();
};