#ifndef __TRANS_H__
#define __TRANS_H__

#include "geometry.h"

Mat4x4f viewport(int x,int y,int w,int h,int depth);

Mat4x4f projection(const Vec3f& camera);

Mat4x4f modelview(const Vec3f& camera,const Vec3f& center,const Vec3f& up);

#endif