#include "trans.h"

Mat4x4f viewport(int x,int y,int w,int h,int depth)
{
	auto m=Mat4x4f::identity();
	m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
	return m;
}

Mat4x4f projection(const Vec3f& camera)
{
    Mat4x4f Projection = Mat4x4f::identity();
	Projection[3][2] = -1.f/camera.z();
    return Projection;
}

Mat4x4f modelview(const Vec3f& camera,const Vec3f& center,const Vec3f& up)
{
    Vec3f z=(camera-center).normalized();
    Vec3f x=up.cross_product(z).normalized();
    Vec3f y=z.cross_product(x).normalized();
    Mat4x4f res(x.concat_right(y).concat_right(z).transpose()),Tr=Mat4x4f::identity();
    for(int i=0;i<3;i++)
    {
        Tr[i][3]=-camera[i][0];
        res[3][i]=0;
    }
    res[3][3]=1;
    return res*Tr;
}
