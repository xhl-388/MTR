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

Mat4x4f lookat(const Vec3f& camera,const Vec3f& center,const Vec3f& up)
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

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
{
	bool steep = false;
	if (std::abs(p0.x() - p1.x()) < std::abs(p0.y() - p1.y()))
	{
		std::swap(p0.x(), p0.y());
		std::swap(p1.x(), p1.y());
		steep = true;
	}
	if (p0.x() > p1.x())
	{
		std::swap(p0, p1);
	}
	int dx = p1.x() - p0.x();
	int dy = p1.y() - p0.y();
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = p0.y();
	for (int x = p0.x(); x <= p1.x(); x++)
	{
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error2 += derror2;
		if (error2 > dx)
		{
			y += p1.y() > p0.y() ? 1 : -1;
			error2 -= dx * 2;
		}
	}
}

Vec3f barycentric(Vec3f *pts, Vec3f P)
{
	Vec3f u = Vec3f{pts[1].x() - pts[0].x(), pts[2].x() - pts[0].x(), pts[0].x() - P.x()}.cross_product(
		Vec3f{pts[1].y() - pts[0].y(), pts[2].y() - pts[0].y(), pts[0].y() - P.y()});
	if (std::abs(u.z()) < 1e-2)
		return Vec3f{-1, 1, 1};
	return Vec3f{1.f - (u.x() + u.y()) / u.z(), u.x() / u.z(), u.y() / u.z()};
}

void triangle(Vec3f *pts, TGAImage &image, Vec3f* uvs, float intensity ,TGAImage &zbuffer)
{
	Vec2f bboxmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	Vec2f bboxmax{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	Vec2f clamp{ image.get_width() - 1, image.get_height() - 1};
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j][0] = std::max(0.f, std::min(bboxmin[j][0], pts[i][j][0]));
			bboxmax[j][0] = std::min(clamp[j][0], std::max(bboxmax[j][0], pts[i][j][0]));
		}
	}
	Vec3f P;
	for (P.x() = bboxmin.x(); P.x() <= bboxmax.x(); P.x()++)
	{
		for (P.y() = bboxmin.y(); P.y() <= bboxmax.y(); P.y()++)
		{
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x() < 0 || bc_screen.y() < 0 || bc_screen.z() < 0)
				continue;
			P.z() = 0;
			for (int i = 0; i < 3; i++)
				P.z() += bc_screen[i][0] * pts[i].z();
			if (zbuffer.get(P.x(),P.y())[0]< P.z())
			{
				Vec3f uv=uvs[0]*bc_screen.x() + uvs[1]*bc_screen.y() + uvs[2]*bc_screen.z(); 
				zbuffer.get(P.x(),P.y())[0] = P.z();
				image.set(P.x(), P.y(),TGAColor(255,255,255,255)*intensity);
			}
		}
	}
}