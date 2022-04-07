#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "trans.h"

Model *model = NULL;
float* zbuffer;

constexpr int depth=255;
constexpr int width = 1000;
constexpr int height = 1000;

Vec3f light_dir = Vec3f{1,1,1}.normalized();
Vec3f camera{1,1,3};
Vec3f center{0,0,0};
Vec3f up{0,1,0};

struct GouraudShader : public IShader{
	Mat<float,2,3> varying_uv; 
	Mat<float,4,3> varying_tri; // clip space triangle vertex coordinates
	Mat<float,3,3> ndc_tri;		// triangle vertices' NDC coordinates
	Mat<float,3,3> varying_nrm;	// original rough per vertex normal in .obj
	Mat4x4f uniform_M;   //  Projection*ModelView
    Mat4x4f uniform_MIT; // (Projection*ModelView).invert_transpose()

	virtual Vec4f vertex(int iface, int nthvert){
		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		varying_nrm.set_col(nthvert,Mat<float,3,1>(uniform_MIT*Vec4f(model->normal(iface,nthvert),0)));
		Vec4f gl_vertex=uniform_M*Vec4f(model->vert(iface,nthvert),1);
		varying_tri.set_col(nthvert,gl_vertex);
		ndc_tri.set_col(nthvert,Mat<float,3,1>(gl_vertex/gl_vertex.w()));
		return gl_vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color){
		Vec3f bn=(varying_nrm*bar).normalized();
		Vec2f uv=varying_uv*bar;

		Mat<float,3,3> A;
		A.set_row(0,(ndc_tri.get_col(1)-ndc_tri.get_col(0)).transpose());
		A.set_row(1,(ndc_tri.get_col(2)-ndc_tri.get_col(0)).transpose());
		A.set_row(2,bn.transpose());

		Mat<float,3,3> AI=A.inverse();

		Vec3f i = AI * Vec3f{varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0};
        Vec3f j = AI * Vec3f{varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0};

		Mat<float,3,3> B;
		B.set_col(0,i.normalized());
		B.set_col(1,j.normalized());
		B.set_col(2,bn);

		Vec3f n = Vec3f(uniform_MIT*Vec4f(B*model->normal(uv),1)).normalized();
		Vec3f l = Vec3f(uniform_M*Vec4f(light_dir,1)).normalized();
		Vec3f r = (n*(n.product(l)*2.f)-l).normalized();
		float spec = pow(std::max(0.f,r.z()), model->specular(uv));
		float diffuse = std::max(0.f,n.product(l));
		TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diffuse + .6*spec), 255);
        return false;
	}
};

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	zbuffer = new float[width*height];
	std::fill(zbuffer,zbuffer+width*height,-std::numeric_limits<float>::max());

	if (2 == argc){
		model = new Model(argv[1]);
	}else{
		model = new Model("../obj/african_head.obj");
	}

	ModelView = lookat(camera,center,up);
	Projection = projection(-1.f/(camera-center).norm());
	Viewport   = viewport(0, 0, width, height,depth);

	GouraudShader shader;
	shader.uniform_M=Projection*ModelView;
	shader.uniform_MIT=shader.uniform_M.inverse().transpose();
	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++){
			screen_coords[j] = shader.vertex(i,j);
		}
		triangle(shader.varying_tri, image, shader, zbuffer);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	std::cout<<"Reach the end!"<<std::endl;

	delete model;
	return 0;
}
