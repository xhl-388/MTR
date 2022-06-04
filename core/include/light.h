#pragma once

#include <matrix.h>

struct Light {
    Vec3f origin;
    Vec3f dir;
    Light(const Vec3f& _origin, const Vec3f& _pos):origin(_origin), dir(_pos) {}
    Light(const Light& li): origin(li.origin), dir(li.dir) {
    }
};