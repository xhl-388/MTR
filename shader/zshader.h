#pragma once

#include <ishader.h>
#include <scene.h>

struct ZShader : public IShader {
    Scene* scene;

    Mat<float,4,3> varying_tri;

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = scene->m_proMat*scene->GetViewMatrix()*Vec4f(scene->m_model->vert(iface, nthvert),1);
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment( Vec3f bar, TGAColor &color) {
        color = TGAColor(0, 0, 0);
        return false;
    }
};