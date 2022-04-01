#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "trans.h"

Model *model = NULL;

constexpr int depth=255;

Vec3f light_dir{0,0,-1};
Vec3f camera{0,0,2};

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width ,height ,TGAImage::GRAYSCALE);

	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
	}

	Mat4x4f ModelView = lookat(camera,Vec3f{0,0,0},Vec3f{0,1,0});
	Mat4x4f Projection = projection(camera);
	Mat4x4f ViewPort   = viewport(0, 0, width, height,depth);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec3f uvs[3];
		for (int j = 0; j < 3; j++)
		{
			world_coords[j] = model->vert(i,j);
			Vec4f t(world_coords[j]);
			t[3][0]=1;
			Vec4f v4=ViewPort*Projection*ModelView*t;
			screen_coords[j] = 
				Vec3f{ float(int(v4.x()/v4.w()+0.5f)),float(int(v4.y()/v4.w()+0.5f)),v4.z()/v4.w()};
			uvs[j] = model->uv(face[3*j+1]);
		}
		Vec3f n=((world_coords[2] - world_coords[0]).cross_product(world_coords[1] - world_coords[0])).normalized();
		float intensity = n.product(light_dir);
		if (intensity > 0)
		{
			triangle(screen_coords, image, uvs, intensity, zbuffer);
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output1.tga");

	zbuffer.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbuffer.write_tga_file("zbuffer.tga");

	std::cout<<"Reach the end!"<<std::endl;

	delete model;
	return 0;
}
