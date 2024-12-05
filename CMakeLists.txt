cmake_minimum_required(VERSION 3.22)
project(vk_engine)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR src)
set(EXTERNAL third_party)

# add_compile_definitions(DEBUG)

find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories("${SRC_DIR}")
include_directories("${EXTERNAL}/glm")
include_directories("${EXTERNAL}/stb")
include_directories("${EXTERNAL}/earcut.hpp/include")

set(SHADERS ${SRC_DIR}/backend/render/vk/shaders/simple.vert  ${SRC_DIR}/backend/render/vk/shaders/simple.frag)

foreach (SHADER ${SHADERS})
    get_filename_component(SHADERNAME ${SHADER} LAST_EXT)
    string(SUBSTRING "${SHADERNAME}" 1 -1 SHADERNAME)
    string(TOUPPER ${SHADERNAME} SHADERNAME)
    file(READ ${SHADER} ${SHADERNAME})
endforeach ()

pkg_check_modules(SHADERC REQUIRED shaderc)

configure_file(${SRC_DIR}/backend/render/vk/shaders.cc.in ${SRC_DIR}/backend/render/vk/shaders.cc @ONLY)

add_executable(vk_engine
        ${SRC_DIR}/main.cc
        ${SRC_DIR}/app/app.cc
        ${SRC_DIR}/app/app.h
        ${SRC_DIR}/obj/parser.h
        ${SRC_DIR}/obj/parser.cc
        ${SRC_DIR}/backend/render/vk/config.cc
        ${SRC_DIR}/backend/render/vk/config.h
        ${SRC_DIR}/backend/render/vk/render.cc
        ${SRC_DIR}/backend/render/vk/render.h
        ${SRC_DIR}/backend/render/vk/error.h
        ${SRC_DIR}/backend/render/vk/dispatchable.h
        ${SRC_DIR}/backend/render/vk/object.h
        ${SRC_DIR}/backend/render/vk/object.cc
        ${SRC_DIR}/backend/render/vk/commander.h
        ${SRC_DIR}/backend/render/vk/commander.cc
        ${SRC_DIR}/backend/render/vk/instance.h
        ${SRC_DIR}/backend/render/vk/instance.cc
        ${SRC_DIR}/backend/render/vk/device.h
        ${SRC_DIR}/backend/render/vk/device.cc
        ${SRC_DIR}/backend/render/vk/commander.h
        ${SRC_DIR}/backend/render/vk/commander.cc
        ${SRC_DIR}/backend/render/vk/shaders.h
        ${SRC_DIR}/backend/render/vk/shaders.cc

        ${SRC_DIR}/backend/window/glfw.h
        ${SRC_DIR}/backend/window/glfw.cc
)

target_link_directories(vk_engine PUBLIC
        ${SHADERC_LIBRARY_DIRS}
)

target_link_libraries(vk_engine
        Vulkan::Vulkan
        glfw
        ${SHADERC_LIBRARIES}
)