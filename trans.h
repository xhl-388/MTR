#ifndef __TRANS_H__
#define __TRANS_H__

#include "geometry.h"
#include "tgaimage.h"
#include <limits>
#include <omp.h>

extern Mat4x4f Viewport;
extern Mat4x4f Projection;
extern Mat4x4f ModelView;

struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

Mat4x4f viewport(int x,int y,int w,int h,int depth);

Mat4x4f projection(float coeff);

Mat4x4f lookat(const Vec3f& camera,const Vec3f& center,const Vec3f& up);

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);

void triangle(Mat<float,4,3>& clipc, TGAImage &image, IShader& shader, float* zbuffer);

#endif