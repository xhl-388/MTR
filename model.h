#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "tgaimage.h"
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> uvs_;
	std::vector<Vec3f> norms_;
	std::vector<std::vector<int> > faces_;
	TGAImage diffusemap_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int iface,int nthvert);
	Vec3f uv(int i);
	Vec3f norm(int i);
	TGAColor diffuse(Vec2f uv) const;
	std::vector<int> face(int idx);
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
};

#endif //__MODEL_H__