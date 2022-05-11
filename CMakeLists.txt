cmake_minimum_required(VERSION 3.15)

project(mtr VERSION 0.1.0 LANGUAGES CXX)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)

file(GLOB mtr_sources *.cpp *.h)
add_executable(mtr ${mtr_sources})

find_package(OpenMP REQUIRED)
target_link_libraries(mtr PUBLIC OpenMP::OpenMP_CXX)
