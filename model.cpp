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
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        }else if(!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vec3f vh;
            for(int i=0;i<3;i++) iss >> vh.raw[i];
            uvs_.push_back(vh);
        }else if(!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            Vec3f vn;
            for(int i=0;i<3;i++) iss >> vn.raw[i];
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
    std::cerr << "# v# " << verts_.size() << "# vh# " <<uvs_.size() << " f# "  << faces_.size() << std::endl;
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

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::uv(int i) {
    return uvs_[i];
}

Vec3f Model::norm(int i){
    return norms_[i];
}