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
Vec3f camera{1,0,1};
Vec3f center{0,0,0};
Vec3f up{0,1,0};

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width ,height ,TGAImage::GRAYSCALE);

	if (2 == argc){
		model = new Model(argv[1]);
	}else{
		model = new Model("obj/african_head.obj");
	}

	Mat4x4f ModelView = lookat(camera,center,up);
	Mat4x4f Projection = projection(-1.f/(camera-center).norm());
	Mat4x4f ViewPort   = viewport(0, 0, width, height,depth);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec4f screen_coords[3];
		Vec3f world_coords[3];
		Vec3f uvs[3];
		for (int j = 0; j < 3; j++)
		{
			world_coords[j] = model->vert(i,j);
			Vec4f t(world_coords[j]);
			t[3][0]=1;
			screen_coords[j] =ViewPort*Projection*ModelView*t;
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
	zbuffer.flip_vertically();
	image.write_tga_file("output1.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	std::cout<<"Reach the end!"<<std::endl;

	delete model;
	return 0;
}
