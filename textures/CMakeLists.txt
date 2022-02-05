cmake_minimum_required(VERSION 3.21)
project(textures)

set(CMAKE_CXX_STANDARD 17)

add_executable(textures main.cpp Textures.cpp Textures.h Solution.cpp Common.h)
