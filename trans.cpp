#include "trans.h"

Mat4x4f ModelView;
Mat4x4f Viewport;
Mat4x4f Projection;

IShader::~IShader() {}

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

Mat4x4f projection(float coeff)
{
    Mat4x4f Projection = Mat4x4f::identity();
	Projection[3][2] = coeff;
    return Projection;
}

Mat4x4f lookat(const Vec3f& camera,const Vec3f& center,const Vec3f& up)
{
    Vec3f z=(camera-center).normalized();
    Vec3f x=up.cross_product(z).normalized();
    Vec3f y=z.cross_product(x).normalized();

    Mat4x4f res=Mat4x4f::identity();
	res.assign(x.concat_right(y).concat_right(z).transpose());
    for(int i=0;i<3;i++)
    {
        res[i][3]=-center[i][0];
    }
   	return res;
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

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
	Vec3f u = Vec3f{B.x() - A.x(), C.x() - A.x(), A.x() - P.x()}.cross_product(
		Vec3f{B.y() - A.y(), C.y() - A.y(), A.y() - P.y()});
	if (std::abs(u.z()) < 1e-2)
		return Vec3f{-1, 1, 1};
	return Vec3f{1.f - (u.x() + u.y()) / u.z(), u.x() / u.z(), u.y() / u.z()};
}

void triangle(Mat<float,4,3>& clipc, TGAImage &image, IShader& shader, float* zbuffer)
{
	Mat<float,3,4> pts=(Viewport*clipc).transpose();
	Mat<float,3,2> pts2;
	for(int i=0;i<3;i++)
		pts2.set_row(i,Mat<float,1,2>(pts.get_row(i)/pts[i][3]));
    Vec2f bboxmin{ std::numeric_limits<float>::max(),  std::numeric_limits<float>::max()};
    Vec2f bboxmax{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	Vec2f clamp{image.get_width()-1, image.get_height()-1};
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j][0] = std::max(0.f,std::min(bboxmin[j][0], pts2[i][j]));
            bboxmax[j][0] = std::min(clamp[j][0], std::max(bboxmax[j][0], pts2[i][j]));
        }
    }
	Vec2i P;
	TGAColor color(255,255,255);
	for (P.x() = bboxmin.x(); P.x() <= bboxmax.x(); P.x()++)
	{
		for (P.y() = bboxmin.y(); P.y() <= bboxmax.y(); P.y()++)
		{
			Vec3f bc_screen = barycentric(pts2.get_row(0).transpose(), pts2.get_row(1).transpose(), 
				pts2.get_row(2).transpose(), Vec2f(P));
			Vec3f bc_clip = Vec3f{bc_screen.x()/pts[0][3], bc_screen.y()/pts[1][3], 
				bc_screen.z()/pts[2][3]};
			bc_clip = bc_clip/(bc_clip.x()+bc_clip.y()+bc_clip.z());
			float frag_depth=clipc.get_row(2).transpose().product(bc_clip);
			
			if (bc_screen.x() < 0 || bc_screen.y() < 0 || bc_screen.z() < 0 ||
				zbuffer[P.x()+P.y()*image.get_width()]> frag_depth)
				continue;
			bool discard = shader.fragment(bc_clip,color);
			if (!discard)
			{
                zbuffer[P.x()+P.y()*image.get_width()] = frag_depth;
				image.set(P.x(), P.y(), color);
			}
		}
	}
}