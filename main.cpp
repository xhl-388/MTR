#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include <api.h>
#include <show.h>
#include <utils.h>

Model *model = NULL;
float* zbuffer;
float* shadowbuffer;

constexpr int depth=255;
constexpr int width = 1000;
constexpr int height = 1000;

Vec3f light_pos = Vec3f{-1,1,1};
Vec3f light_dir = Vec3f{-1,1,1}.normalized();
Vec3f camera{1,1,4};
Vec3f center{0,0,0};
Vec3f up{0,1,0};

struct GouraudShader : public IShader{
	TGAImage* ao_tex;
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
		TGAColor ambient = ao_tex->get(uv.x()*width,uv.y()*height);
        for (int i=0; i<3; i++) color[i] = std::min<float>(c[i]*shadow*(1.2*diffuse + .6*spec+ ambient[i]/255.f*0.3), 255);
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

struct ZShader : public IShader {
    Mat<float,4,3> varying_tri;

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = Projection*ModelView*Vec4f(model->vert(iface, nthvert),1);
        varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment( Vec3f bar, TGAColor &color) {
        color = TGAColor(0, 0, 0);
        return false;
    }
};

float max_elevation_angle(float *buffer, Vec2f p, Vec2f dir) {
    float maxangle = 0;
    for (float t=0.; t<10.; t+=1.) {
        Vec2f cur = p + dir*t;
        if (cur.x()>=width || cur.y()>=height || cur.x()<0 || cur.y()<0
			||buffer[int(cur.x())+int(cur.y())*width]==-std::numeric_limits<float>::max()) 
			return maxangle;
        float distance = (p-cur).norm();
        if (distance < 1.f) continue;
        float elevation = (buffer[int(cur.x())+int(cur.y())*width]-buffer[int(p.x())+int(p.y())*width])/depth;
		// std::cout<<elevation<<std::endl;
        maxangle = std::max(maxangle, atanf(elevation/distance));
    }
    return maxangle;
}

void key_callback(window_t *window, keycode_t key, int pressed) {
	if(key == KEY_SPACE && pressed == 1) {
        window->should_close = true;
    }
}

void test_show_keys_input(window_t* window) {
	printf("%d %d %d %d %d \n",window->keys[0],
        window->keys[1],
        window->keys[2],
        window->keys[3],
        window->keys[4]
    );
}

int main(int argc, char **argv)
{
	// prepare data
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage depthImg(width,height,TGAImage::GRAYSCALE);
	TGAImage frame(width, height, TGAImage::RGB);
	
	zbuffer = new float[width*height];
	std::fill(zbuffer,zbuffer+width*height,-std::numeric_limits<float>::max());
	shadowbuffer = new float[width*height];
	std::fill(shadowbuffer,shadowbuffer+width*height,-std::numeric_limits<float>::max());

	const char* outputDir;
	if (3 == argc){
		model = new Model(argv[1]);
		outputDir = argv[2];
	}else{
		std::cerr << "Function main(): 3 args expected, " << argc <<" args provided !" << std::endl;
		exit(-1);
	}

	// first pass : shadow pass
	{
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
		depthImg.write_tga_file(FileNameConcat(outputDir, "depth.tga"));
	}

	{
		ModelView = lookat(camera,center,up);
		Projection = projection(-1.f/(camera-center).norm());
		Viewport   = viewport(0, 0, width, height,depth);

		ZShader zshader;
		for (int i=0; i<model->nfaces(); i++) {
			for (int j=0; j<3; j++) {
				zshader.vertex(i, j);
			}
			triangle(zshader.varying_tri, frame, zshader, zbuffer);
		}
		
		omp_set_num_threads(20);
		#pragma omp parallel for
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				if (zbuffer[x+y*width] < -1e5) continue;
				float total = 0;
				for (float a=0; a<M_PI*2-1e-4; a += M_PI/4) {
					total += M_PI/2 - max_elevation_angle(zbuffer, Vec2f{x, y}, 
						Vec2f{std::cos(a), std::sin(a)});
				}
				total /= (M_PI/2)*8;
				total = std::pow(total, 100.f);
				int val = std::min(255,int(total*255));
				frame.set(x, y, TGAColor(val,val,val));
			}
		}
		frame.flip_vertically();
		frame.write_tga_file(FileNameConcat(outputDir, "framebuffer.tga"));
		std::fill(zbuffer,zbuffer+width*height,-std::numeric_limits<float>::max());
	}

	Mat4x4f M = Viewport*Projection*ModelView;

	// second pass : render pass
	{
		ModelView = lookat(camera,center,up);
		Projection = projection(-1.f/(camera-center).norm());
		Viewport   = viewport(0, 0, width, height,depth);

		GouraudShader shader;
		shader.uniform_M=Projection*ModelView;
		shader.uniform_MIT=shader.uniform_M.inverse().transpose();
		shader.uniform_Mshadow = M*(shader.uniform_M.inverse());
		shader.ao_tex = &frame;
		for (int i = 0; i < model->nfaces(); i++)
		{
			for (int j = 0; j < 3; j++){
				shader.vertex(i,j);
			}
			triangle(shader.varying_tri, image, shader, zbuffer);
		}

		image.flip_vertically();
		image.write_tga_file(FileNameConcat(outputDir, "output.tga"));
	}
	std::cout<<"Reach the end!"<<std::endl;

	image.flip_vertically();
	
	window_t* win = window_create("Render test", width, height);

	image_t * targetImg = image_create(width, height, 3);
	targetImg->buffer = image.buffer();
	callbacks_t callbacks;
	callbacks.button_callback = nullptr;
	callbacks.key_callback = key_callback;
	callbacks.scroll_callback = nullptr;
	input_set_callbacks(win, callbacks);
	while(!window_should_close(win)) {

		// ModelView = lookat(camera,center,up);
		// Projection = projection(-1.f/(camera-center).norm());
		// Viewport   = viewport(0, 0, width, height,depth);

		// GouraudShader shader;
		// shader.uniform_M=Projection*ModelView;
		// shader.uniform_MIT=shader.uniform_M.inverse().transpose();
		// shader.uniform_Mshadow = M*(shader.uniform_M.inverse());
		// shader.ao_tex = &frame;
		// for (int i = 0; i < model->nfaces(); i++)
		// {
		// 	for (int j = 0; j < 3; j++){
		// 		shader.vertex(i,j);
		// 	}
		// 	triangle(shader.varying_tri, image, shader, zbuffer);
		// }
		window_draw_image(win, targetImg);
		test_show_keys_input(win);
		input_poll_events();
	}

	window_destroy(win);

	delete model;
	return 0;
}
