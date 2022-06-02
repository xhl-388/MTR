#ifndef __TRANS_H__
#define __TRANS_H__

#include <limits>
#include <omp.h>
#include <ishader.h>
#include <matrix.h>

extern Mat4x4f Viewport;
extern Mat4x4f Projection;
extern Mat4x4f ModelView;


Mat4x4f viewport(int x,int y,int w,int h,int depth);

Mat4x4f projection(float coeff);

Mat4x4f lookat(const Vec3f& camera,const Vec3f& center,const Vec3f& up);

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);

void triangle(Mat<float,4,3>& clipc, TGAImage &image, IShader& shader, float* zbuffer);

#endif