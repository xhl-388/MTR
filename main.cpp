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
constexpr int width = 800;
constexpr int height = 800;

Vec3f light_dir = Vec3f{1,1,1}.normalized();
Vec3f camera{1,1,3};
Vec3f center{0,0,0};
Vec3f up{0,1,0};

struct GouraudShader : public IShader{
	Vec3f varying_intensity;
	Mat<float,2,3> varying_uv; 

	virtual Vec4f vertex(int iface, int nthvert){
		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		varying_intensity[nthvert][0] = std::max(0.f, model->norm(iface,nthvert).product(light_dir));
		Vec4f gl_vertex(model->vert(iface,nthvert));
		gl_vertex.w()=1;
		return Viewport*Projection*ModelView*gl_vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color){
		float intensity = varying_intensity.product(bar);
		Vec2f uv=varying_uv*bar;
		color = model->diffuse(uv)*intensity;
		return false;
	}
};

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width ,height ,TGAImage::GRAYSCALE);

	if (2 == argc){
		model = new Model(argv[1]);
	}else{
		model = new Model("obj/african_head.obj");
	}

	ModelView = lookat(camera,center,up);
	Projection = projection(-1.f/(camera-center).norm());
	Viewport   = viewport(0, 0, width, height,depth);

	GouraudShader shader;
	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++){
			screen_coords[j] = shader.vertex(i,j);
		}
		triangle(screen_coords, image, shader, zbuffer);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbuffer.flip_vertically();
	image.write_tga_file("output1.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	std::cout<<"Reach the end!"<<std::endl;

	delete model;
	return 0;
}
