#pragma once

#include <ishader.h>
#include <scene.h>

struct DepthShader : public IShader {
    Scene* scene;

    Mat<float,4,3> varying_tri;

    DepthShader() : varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex =scene->m_proMat*scene->GetViewMatrix()*Vec4f(scene->m_model->vert(iface, nthvert),1); // read the vertex from .obj file
        varying_tri.set_col(nthvert, gl_Vertex/gl_Vertex.w());
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec4f pt = varying_tri*bar;
		Vec3f p = Vec3f(pt/pt.w());
        color = TGAColor(255, 255, 255)*(p.z()/2+0.5f);
        return false;
    }
};