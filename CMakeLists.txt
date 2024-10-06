cmake_minimum_required(VERSION 3.22)
project(vk_engine)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR src)
set(EXTERNAL third_party)

set(SHADER_MAX_TEXTURE 14)
add_subdirectory(shaders)
add_compile_definitions(MAX_TEXTURE_COUNT=${SHADER_MAX_TEXTURE})
# add_compile_definitions(DEBUG)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories("${SRC_DIR}")
include_directories("${EXTERNAL}/glm")
include_directories("${EXTERNAL}/stb")
include_directories("${EXTERNAL}/earcut.hpp/include")

add_executable(vk_engine
        ${SRC_DIR}/main.cc
        ${SRC_DIR}/app/app.cc
        ${SRC_DIR}/app/app.h
        ${SRC_DIR}/base/io.cc
        ${SRC_DIR}/base/io.h
        ${SRC_DIR}/obj/parser.h
        ${SRC_DIR}/obj/parser.cc
        ${SRC_DIR}/backend/render/vk_config.cc
        ${SRC_DIR}/backend/render/vk_config.h
        ${SRC_DIR}/backend/render/vk_commander.cc
        ${SRC_DIR}/backend/render/vk_commander.h
        ${SRC_DIR}/backend/render/vk_backend.cc
        ${SRC_DIR}/backend/render/vk_backend.h
        ${SRC_DIR}/backend/render/vk_factory.cc
        ${SRC_DIR}/backend/render/vk_factory.h
        ${SRC_DIR}/backend/render/vk_types.cc
        ${SRC_DIR}/backend/render/vk_types.h
        ${SRC_DIR}/backend/render/vk_object.h
        ${SRC_DIR}/backend/render/vk_object.cc
        ${SRC_DIR}/backend/render/vk_wrappers.h
        ${SRC_DIR}/backend/render/vk_wrappers.cc
        ${SRC_DIR}/backend/window/glfw.h
        ${SRC_DIR}/backend/window/glfw.cc
)

target_link_libraries(vk_engine Vulkan::Vulkan glfw)