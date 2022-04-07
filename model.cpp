#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <cassert>
#include "model.h"

Model::Model(const char *filename) : verts_(),uvs_(),norms_(),faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        // vertex detected
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.at(i);
            verts_.push_back(v);
        }else if(!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vec3f vh;
            for(int i=0;i<3;i++) iss >> vh.at(i);
            uvs_.push_back(Vec2f(vh));
        }else if(!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            Vec3f vn;
            for(int i=0;i<3;i++) iss >> vn.at(i);
            norms_.push_back(vn);
        }else if (!line.compare(0, 2, "f ")) { // face detected
            std::vector<int> f;
            int tmp[3];
            iss >> trash;
            // the obj format of face presentation is "f v1i/v2i/v3i v1i/v2i/v3i "
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for(int i=0;i<3;i++)
                    f.push_back(tmp[i]-1);  // in wavefront obj all indices start at 1, not zero
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << "# vh# " <<uvs_.size()
    << "# vn# " << norms_.size()<< " f# "  << faces_.size() << std::endl;
    load_texture(filename,"_diffuse.tga",diffusemap_);
    load_texture(filename, "_nm_tangent.tga",   normalmap_);
    load_texture(filename, "_spec.tga" ,specularmap_);
}

Model::~Model() {
}

int Model::nverts() {
    assert(verts_.size()<=static_cast<size_t>(std::numeric_limits<int>::max()));
    return (int)verts_.size();
}

int Model::nfaces() {
    assert(faces_.size()<=static_cast<size_t>(std::numeric_limits<int>::max()));
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) const{
    return faces_[idx];
}

Vec3f Model::vert(int iface,int nthvert) const{
    return verts_[faces_[iface][3*nthvert]];
}

Vec2f Model::uv(int iface, int nthvert) const{
    return uvs_[faces_[iface][3*nthvert+1]];
}

Vec3f Model::normal(int iface, int nthvert) const{
    return norms_[faces_[iface][3*nthvert+2]].normalized();
}

Vec3f Model::normal(Vec2f uvf) const{
    Vec2i uv{uvf.x()*normalmap_.get_width(), uvf.y()*normalmap_.get_height()};
    TGAColor c = normalmap_.get(uv.x(), uv.y());
    Vec3f res;
    for (int i=0; i<3; i++)
        res[2-i][0] = (float)c[i]/255.f*2.f - 1.f;
    return res;
}

TGAColor Model::diffuse(Vec2f uv) const
{
    return diffusemap_.get(diffusemap_.get_width()*uv.x(),diffusemap_.get_height()*uv.y());
}

float Model::specular(Vec2f uvf) const
{
    Vec2i uv{uvf.x()*specularmap_.get_width(), uvf.y()*specularmap_.get_height()};
    return specularmap_.get(uvf.x(), uvf.y())[0];
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}