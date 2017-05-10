cmake_minimum_required (VERSION 2.8)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Weverything -Wno-c++98-compat -Wno-padded -Wno-c++98-compat-pedantic -g")
set(CMAKE_LD_FLAGS "-g")

project(deviser)

add_executable(deviser src/main.cpp
                       src/deviser.cpp)
