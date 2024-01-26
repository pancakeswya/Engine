cmake_minimum_required(VERSION 3.10)

project(CreatingWindow)

set(CMAKE_PREFIX_PATH "C:/Program Files (x86)/GLFW/lib/cmake/glfw3")
set(CMAKE_CXX_STANDARD 17)

include_directories("C:/Program Files (x86)/GLFW/include/GLFW/")
include_directories("src")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(DataParser
        src/base/data_parser.h
        src/base/data_parser.cc)

add_library(MeshMaker
        src/base/mesh_maker.h
        src/base/mesh_maker.cc)

target_link_libraries(MeshMaker DataParser)

add_library(MeshModel
        src/model/mesh_model.h
        src/model/mesh_model.cc)

target_link_libraries(MeshModel MeshMaker)

add_library(CameraModel
        src/model/camera_model.h
        src/model/camera_model.cc)

add_library(Application
        src/view/application.h
        src/view/application.cc
)

target_link_libraries(Application glfw3)

add_library(Renderer
        src/view/renderer.h
        src/view/renderer.cc
)

target_link_libraries(Renderer MeshModel CameraModel opengl32)

add_library(Window
        src/view/window.h
        src/view/window.cc
)

target_link_libraries(Window Renderer glfw3)

add_executable(Engine
        src/engine.h
        src/engine.cc
        src/main.cc
)

target_link_libraries(Engine Application Window)