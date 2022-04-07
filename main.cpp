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
float* shadowbuffer;

constexpr int depth=255;
constexpr int width = 1000;
constexpr int height = 1000;

Vec3f light_pos = Vec3f{0,1,1};
Vec3f light_dir = Vec3f{0,1,1}.normalized();
Vec3f camera{1,1,4};
Vec3f center{0,0,0};
Vec3f up{0,1,0};

struct GouraudShader : public IShader{
	Mat<float,2,3> varying_uv; 
	Mat<float,4,3> varying_tri; // clip space triangle vertex coordinates
	Mat<float,3,3> ndc_tri;		// triangle vertices' NDC coordinates
	Mat<float,3,3> varying_nrm;	// original rough per vertex normal in .obj
	Mat4x4f uniform_M;   //  Projection*ModelView
    Mat4x4f uniform_MIT; // (Projection*ModelView).invert_transpose()
	Mat4x4f uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates

	virtual Vec4f vertex(int iface, int nthvert){
		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		varying_nrm.set_col(nthvert,Mat<float,3,1>(uniform_MIT*Vec4f(model->normal(iface,nthvert),0)));
		Vec4f gl_vertex=uniform_M*Vec4f(model->vert(iface,nthvert),1);
		varying_tri.set_col(nthvert,gl_vertex);
		ndc_tri.set_col(nthvert,Mat<float,3,1>(gl_vertex/gl_vertex.w()));
		return gl_vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color){
		Vec4f sb_p = uniform_Mshadow*varying_tri*bar; // corresponding point in the shadow buffer
        sb_p = sb_p/sb_p.w();
        int idx = int(sb_p.x()) + int(sb_p.y())*width; // index in the shadowbuffer array
        float shadow = .3+.7*(shadowbuffer[idx]<sb_p.z()+43.34); // magic coeff to avoid z-fighting

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

		Vec3f n = (B*model->normal(uv)).normalized();
		Vec3f l = light_dir;
		Vec3f r = (n*(n.product(l)*2.f)-l).normalized();
		float spec = pow(std::max(0.f,r.z()), model->specular(uv));
		float diffuse = std::max(0.f,n.product(l));
		TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(20 + c[i]*shadow*(1.2*diffuse + .6*spec), 255);
        return false;
    }
};

struct DepthShader : public IShader {
    Mat<float,4,3> varying_tri;

    DepthShader() : varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex =Projection*ModelView*Vec4f(model->vert(iface, nthvert),1); // read the vertex from .obj file
        varying_tri.set_col(nthvert, gl_Vertex/gl_Vertex.w());
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec4f pt = varying_tri*bar;
		Vec3f p = Vec3f(pt/pt.w());
        color = TGAColor(255, 255, 255)*(p.z()/2+0.5f);
        return false;
    }
};

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage depthImg(width,height,TGAImage::GRAYSCALE);
	zbuffer = new float[width*height];
	std::fill(zbuffer,zbuffer+width*height,-std::numeric_limits<float>::max());
	shadowbuffer = new float[width*height];
	std::fill(shadowbuffer,shadowbuffer+width*height,-std::numeric_limits<float>::max());

	if (2 == argc){
		model = new Model(argv[1]);
	}else{
		model = new Model("../obj/diablo3_pose.obj");
	}

	ModelView = lookat(light_pos,center,up);
	Projection = projection(0);
	Viewport = viewport(0,0,width,height,depth);

	DepthShader depthShader;

	for (int i = 0; i < model->nfaces(); i++)
	{
		for (int j = 0; j < 3; j++){
			depthShader.vertex(i,j);
		}
		triangle(depthShader.varying_tri, depthImg, depthShader, shadowbuffer);
	}
	depthImg.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	depthImg.write_tga_file("depth.tga");

	Mat4x4f M = Viewport*Projection*ModelView;


	ModelView = lookat(camera,center,up);
	Projection = projection(-1.f/(camera-center).norm());
	Viewport   = viewport(0, 0, width, height,depth);

	GouraudShader shader;
	shader.uniform_M=Projection*ModelView;
	shader.uniform_MIT=shader.uniform_M.inverse().transpose();
	shader.uniform_Mshadow = M*(shader.uniform_M.inverse());
	for (int i = 0; i < model->nfaces(); i++)
	{
		for (int j = 0; j < 3; j++){
			shader.vertex(i,j);
		}
		triangle(shader.varying_tri, image, shader, zbuffer);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");

	std::cout<<"Reach the end!"<<std::endl;

	delete model;
	return 0;
}
