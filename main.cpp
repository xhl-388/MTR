#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 2000;
const int height = 2000;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
{
	bool steep = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y))
	{
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = p0.y;
	for (int x = p0.x; x <= p1.x; x++)
	{
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error2 += derror2;
		if (error2 > dx)
		{
			y += p1.y > p0.y ? 1 : -1;
			error2 -= dx * 2;
		}
	}
}

Vec3f barycentric(Vec3f *pts, Vec3f P)
{
	Vec3f u = Vec3f(pts[1][0] - pts[0][0], pts[2][0] - pts[0][0], pts[0][0] - P[0]) ^ Vec3f(pts[1][1] - pts[0][1], pts[2][1] - pts[0][1], pts[0][1] - P[1]);
	if (std::abs(u.z) < 1e-2)
		return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, Vec3f* uvs, const TGAImage& texture,float intensity)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
				continue;
			P.z = 0;
			for (int i = 0; i < 3; i++)
				P.z += bc_screen[i] * pts[i].z;
			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				Vec3f uv=uvs[0]*bc_screen.x + uvs[1]*bc_screen.y + uvs[2]*bc_screen.z; 
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y,texture.get(int(uv.x*texture.get_width()),int(uv.y*texture.get_height()))*intensity);
			}
		}
	}
}

Vec3f world2screen(Vec3f v)
{
	return Vec3f(int((v.x + 1.f) * width / 2.f + .5f), int((v.y + 1.f) * height / 2.f + .5f), v.z);
}

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage texture;
	if(!texture.read_tga_file("./obj/african_head_diffuse.tga"))
	{
		std::cerr << "Failed to load the texture! \n" << std::endl;
		exit(-1);
	}
	texture.flip_vertically();

	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
	}

	float *zbuffer = new float[width * height];
	std::fill(zbuffer, zbuffer + width * height, std::numeric_limits<float>::min());

	Vec3f light_dir(0, 0, -1); // define light_dir

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec3f uvs[3];
		for (int j = 0; j < 3; j++)
		{
			world_coords[j] = model->vert(face[3*j]);
			screen_coords[j] = world2screen(world_coords[j]);
			uvs[j] = model->uv(face[3*j+1]);
		}
		Vec3f n = ((world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0])).normalize();
		float intensity = n * light_dir;
		if (intensity > 0)
		{
			triangle(screen_coords, zbuffer, image, uvs, texture ,intensity);
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}
