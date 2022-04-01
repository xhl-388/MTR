#ifndef __TRANS_H__
#define __TRANS_H__

#include "geometry.h"
#include "tgaimage.h"
#include <limits>

constexpr int width = 800;
constexpr int height = 800;

struct IShader {
    virtual ~IShader();
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

Mat4x4f viewport(int x,int y,int w,int h,int depth);

Mat4x4f projection(float coeff);

Mat4x4f lookat(const Vec3f& camera,const Vec3f& center,const Vec3f& up);

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

Vec3f barycentric(Vec3f *pts, Vec3f P);

void triangle(Vec4f *pts, TGAImage &image, Vec3f* uvs, float intensity, TGAImage &zbuffer);

#endif