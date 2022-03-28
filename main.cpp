#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255); 
// Model *model = NULL;
const int width  = 200;
const int height = 200;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) { 
	bool steep=false;
	if(std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y))
	{
		std::swap(p0.x,p0.y);
		std::swap(p1.x,p1.y);
		steep=true;
	}
	if(p0.x>p1.x)
	{
		std::swap(p0,p1);
	}
	int dx=p1.x-p0.x;
	int dy=p1.y-p0.y;
	int derror2=std::abs(dy)*2;
	int error2=0;
	int y=p0.y;
	for(int x=p0.x;x<=p1.x;x++)
	{
		if(steep)
			image.set(y,x,color);
		else
			image.set(x,y,color);
		error2+=derror2;
		if(error2>dx)
		{
			y+=p1.y>p0.y?1:-1;
			error2-=dx*2;
		}
	}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
	if(t0.y==t1.y&&t0.y==t2.y)	// not triangle
		return;
	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
    if (t0.y>t1.y) std::swap(t0, t1); 
    if (t0.y>t2.y) std::swap(t0, t2); 
    if (t1.y>t2.y) std::swap(t1, t2); 
	// line sweep
	int th=t2.y-t0.y;
	for(int i=0;i<th;i++)
	{
		bool up_part=(i<=t2.y-t1.y);
		int sh=up_part?t2.y-t1.y:t1.y-t0.y;
		float alpha=static_cast<float>(i)/th;
		float beta=(up_part?i:th-i)/static_cast<float>(sh);
		Vec2i A=(t0-t2)*alpha+t2;
		Vec2i B=up_part?(t1-t2)*beta+t2:(t1-t0)*beta+t0;
		if(A.x>B.x)
			std::swap(A,B);
		for(int x=A.x;x<=B.x;x++)
			image.set(x,t2.y-i,color);
	}
}

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
	// if (2==argc) {
    //     model = new Model(argv[1]);
    // } else {
    //     model = new Model("obj/african_head.obj");
    // }

    // TGAImage image(width, height, TGAImage::RGB);
    // for (int i=0; i<model->nfaces(); i++) {
    //     std::vector<int> face = model->face(i);
    //     for (int j=0; j<3; j++) {
    //         Vec3f v0 = model->vert(face[j]);
    //         Vec3f v1 = model->vert(face[(j+1)%3]);
    //         int x0 = (v0.x+1.)*width/2.;
    //         int y0 = (v0.y+1.)*height/2.;
    //         int x1 = (v1.x+1.)*width/2.;
    //         int y1 = (v1.y+1.)*height/2.;
    //         line(x0, y0, x1, y1, image, white);
    //     }
    // }
	Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    // delete model;
    return 0;
}

