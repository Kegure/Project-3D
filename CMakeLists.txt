cmake_minimum_required(VERSION 3.28)
project(untitled4)

set(CMAKE_CXX_STANDARD 17)


add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(untitled4 -lglut -lglfw -lGLEW -lGL -lGLU -lSDL2)