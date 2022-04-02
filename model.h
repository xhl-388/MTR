#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "tgaimage.h"
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> uvs_;
	std::vector<Vec3f> norms_;
	std::vector<std::vector<int> > faces_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int iface, int nthvert) const;
	Vec2f uv(int iface, int nthvert) const;
	Vec3f normal(int iface, int nthvert) const;
	Vec3f normal(Vec2f uv) const;
	TGAColor diffuse(Vec2f uv) const;
	std::vector<int> face(int idx) const;
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
};

#endif //__MODEL_H__