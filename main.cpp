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

Vec3f barycentric(Vec2i* pts, Vec2i P)
{
	Vec3f u=Vec3f(pts[2][0]-pts[0][0],pts[1][0]-pts[0][0],pts[0][0]-P[0])^Vec3f(pts[2][1]-pts[0][1],pts[1][1]-pts[0][1],pts[0][1]-P[1]);
	if(std::abs(u.z)<1)
		return Vec3f(-1,1,1);
	return Vec3f(1.f-(u.x+u.y)/u.z,u.x/u.z,u.y/u.z);
}

void triangle(Vec2i* pts, TGAImage& image, TGAColor color)
{
	Vec2i bboxmin(image.get_width()-1,image.get_height()-1);
	Vec2i bboxmax(0,0);
	Vec2i clamp(image.get_width()-1,image.get_height()-1);
	for (int i=0; i<3; i++) { 
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    } 
    Vec2i P; 
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
            image.set(P.x, P.y, color); 
        } 
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

    triangle(t0, image, red);
    triangle(t1, image, white);
    triangle(t2, image, green);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    // delete model;
    return 0;
}

