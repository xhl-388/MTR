#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "geometry.cpp"

Model *model = NULL;
int* zbuffer;

constexpr int width = 800;
constexpr int height = 800;
constexpr int depth=255;

Vec3f light_dir{0,0,-1};
Vec3f camera{0,0,3};

Mat4x4f viewport(int x,int y,int w,int h)
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
	Vec3f u = Vec3f{pts[1].x() - pts[0].x(), pts[2].x() - pts[0].x(), pts[0].x() - P.x()}.crossProduct(
		Vec3f{pts[1].y() - pts[0].y(), pts[2].y() - pts[0].y(), pts[0].y() - P.y()});
	if (std::abs(u.z()) < 1e-2)
		return Vec3f{-1, 1, 1};
	return Vec3f{1.f - (u.x() + u.y()) / u.z(), u.x() / u.z(), u.y() / u.z()};
}

void triangle(Vec3f *pts, TGAImage &image, Vec3f* uvs, float intensity)
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
			if (zbuffer[int(P.x() + P.y() * width)] < P.z())
			{
				Vec3f uv=uvs[0]*bc_screen.x() + uvs[1]*bc_screen.y() + uvs[2]*bc_screen.z(); 
				zbuffer[int(P.x() + P.y() * width)] = P.z();
				image.set(P.x(), P.y(),model->diffuse(Vec2f{uv.x(),uv.y()})*intensity);
			}
		}
	}
}

Vec3f world2screen(Vec3f v)
{
	int x=(v.x() + 1.f) * width / 2.f + .5f;
	int y=(v.y() + 1.f) * height / 2.f + .5f;
	Vec3f res{ float(x) , float(y), (v.z()+1.f)/2*depth};
	return res;;
}

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);

	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
	}

	zbuffer = new int[width * height];
	std::fill(zbuffer, zbuffer + width * height, std::numeric_limits<int>::min());

	Mat4x4f Projection = Mat4x4f::identity();
	Mat4x4f ViewPort   = viewport(width/8, height/8, width*3/4, height*3/4);
	Projection[3][2] = -1.f/camera.z();

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec3f uvs[3];
		for (int j = 0; j < 3; j++)
		{
			world_coords[j] = model->vert(face[3*j]);
			Vec4f t(world_coords[j]);
			t[3][0]=1;
			Vec4f v4=ViewPort*Projection*t;
			screen_coords[j] = 
				Vec3f{ float(int(v4.x()/v4.w()+0.5f)),float(int(v4.y()/v4.w()+0.5f)),v4.z()/v4.w()};
			uvs[j] = model->uv(face[3*j+1]);
		}
		Vec3f n=((world_coords[2] - world_coords[0]).crossProduct(world_coords[1] - world_coords[0])).normalized();
		float intensity = n.product(light_dir);
		if (intensity > 0)
		{
			triangle(screen_coords, image, uvs, intensity);
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output1.tga");

	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i=0; i<width; i++) {
		for (int j=0; j<height; j++) {
			if(zbuffer[i+j*width]!=std::numeric_limits<float>::min())
				zbimage.set(i, j, TGAColor(zbuffer[i+j*width], 1));
		}
	}
	zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbimage.write_tga_file("zbuffer.tga");

	std::cout<<"Reach the end!"<<std::endl;

	delete model;
	delete[] zbuffer;
	return 0;
}
