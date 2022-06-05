cmake_minimum_required(VERSION 3.15)

project(mtr VERSION 0.1.0 LANGUAGES CXX)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

file(GLOB mtr_sources main.cpp core/src/*.cpp core/include/*.h platform/src/*.cpp 
    platform/include/*.h shader/*.h)
add_executable(mtr ${mtr_sources})
target_include_directories(mtr PUBLIC core/include platform/include shader)
target_compile_options(mtr PRIVATE -ffast-math)

find_package(OpenMP REQUIRED)
target_link_libraries(mtr PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(mtr PRIVATE m X11)