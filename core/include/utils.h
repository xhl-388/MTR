#pragma once

#include <cstring>
#include <cstdio>
#include <chrono>

const char* FileNameConcat(const char* dir, const char* filename);

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x) printf("%s: %lfs\n", #x, std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - bench_##x).count());
