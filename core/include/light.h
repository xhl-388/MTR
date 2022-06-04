#pragma once

#include <matrix.h>

struct Light {
    Vec3f origin;
    Vec3f dir;
    Light(const Vec3f& _origin, const Vec3f& _dir):origin(_origin), dir(_dir) {}
    Light(const Light& li): origin(li.origin), dir(li.dir) {
    }
};