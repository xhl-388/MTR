#ifndef __TRANS_H__
#define __TRANS_H__

#include "geometry.h"
#include "tgaimage.h"
#include <limits>

constexpr int width = 800;
constexpr int height = 800;

Mat4x4f viewport(int x,int y,int w,int h,int depth);

Mat4x4f projection(const Vec3f& camera);

Mat4x4f lookat(const Vec3f& camera,const Vec3f& center,const Vec3f& up);

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

Vec3f barycentric(Vec3f *pts, Vec3f P);

void triangle(Vec3f *pts, TGAImage &image, Vec3f* uvs, float intensity, TGAImage &zbuffer);

#endif