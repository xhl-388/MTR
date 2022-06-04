#pragma once

#include <cstring>

const char* FileNameConcat(const char* dir, const char* filename) {
    const size_t len = strlen(dir) + strlen(filename) + 2, dirlen = strlen(dir);
    char* res = new char(len);
    strcpy(res, dir);
    res[dirlen] = '/';
    strcpy(&res[dirlen + 1], filename);
    return res;
}