cmake_minimum_required(VERSION 3.22)
project(vk_engine)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR src)
set(EXTERNAL third_party)

# add_compile_definitions(DEBUG)

find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(SDL2 REQUIRED)

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
#pkg_check_modules(SDL2 IMPORTED_TARGET sdl2)

configure_file(${SRC_DIR}/backend/render/vk/shaders.cc.in ${SRC_DIR}/backend/render/vk/shaders.cc @ONLY)

add_executable(vk_engine
        ${SRC_DIR}/main.cc
        ${SRC_DIR}/app/app.cc
        ${SRC_DIR}/app/app.h
        ${SRC_DIR}/entity/singleton.h
        ${SRC_DIR}/obj/types.h
        ${SRC_DIR}/obj/error.h
        ${SRC_DIR}/obj/parser.h
        ${SRC_DIR}/obj/parser.cc
        ${SRC_DIR}/backend/render/types.h
        ${SRC_DIR}/backend/render/model.h
        ${SRC_DIR}/backend/render/model.cc
        ${SRC_DIR}/backend/render/data_util.cc
        ${SRC_DIR}/backend/render/data_util.h
        ${SRC_DIR}/backend/render/factory.cc
        ${SRC_DIR}/backend/render/factory.h
        ${SRC_DIR}/backend/render/render.h

        ${SRC_DIR}/backend/render/gl/render.cc
        ${SRC_DIR}/backend/render/gl/render.h
        ${SRC_DIR}/backend/render/gl/error.h
        ${SRC_DIR}/backend/render/gl/error.cc

        ${SRC_DIR}/backend/render/vk/config.cc
        ${SRC_DIR}/backend/render/vk/config.h
        ${SRC_DIR}/backend/render/vk/render.cc
        ${SRC_DIR}/backend/render/vk/render.h
        ${SRC_DIR}/backend/render/vk/error.h
        ${SRC_DIR}/backend/render/vk/dispatchable.h
        ${SRC_DIR}/backend/render/vk/object.h
        ${SRC_DIR}/backend/render/vk/object_loader.h
        ${SRC_DIR}/backend/render/vk/object_loader.cc
        ${SRC_DIR}/backend/render/vk/commander.h
        ${SRC_DIR}/backend/render/vk/commander.cc
        ${SRC_DIR}/backend/render/vk/instance.h
        ${SRC_DIR}/backend/render/vk/instance.cc
        ${SRC_DIR}/backend/render/vk/device.h
        ${SRC_DIR}/backend/render/vk/device.cc
        ${SRC_DIR}/backend/render/vk/device_selector.h
        ${SRC_DIR}/backend/render/vk/device_selector.cc
        ${SRC_DIR}/backend/render/vk/commander.h
        ${SRC_DIR}/backend/render/vk/commander.cc
        ${SRC_DIR}/backend/render/vk/shaders.h
        ${SRC_DIR}/backend/render/vk/shaders.cc

        ${SRC_DIR}/backend/window/window.h
        ${SRC_DIR}/backend/window/instance.h
        ${SRC_DIR}/backend/window/factory.h
        ${SRC_DIR}/backend/window/factory.cc

        ${SRC_DIR}/backend/window/glfw/error.h
        ${SRC_DIR}/backend/window/glfw/instance_internal.h
        ${SRC_DIR}/backend/window/glfw/instance_internal.cc
        ${SRC_DIR}/backend/window/glfw/window_internal.h
        ${SRC_DIR}/backend/window/glfw/window_internal.cc
        ${SRC_DIR}/backend/window/glfw/vk/instance.h
        ${SRC_DIR}/backend/window/glfw/vk/window.h
        ${SRC_DIR}/backend/window/glfw/vk/window.cc
        ${SRC_DIR}/backend/window/glfw/gl/instance.h
        ${SRC_DIR}/backend/window/glfw/gl/window.h

        ${SRC_DIR}/backend/window/sdl/error.h
        ${SRC_DIR}/backend/window/sdl/instance_internal.h
        ${SRC_DIR}/backend/window/sdl/instance_internal.cc
        ${SRC_DIR}/backend/window/sdl/window_internal.h
        ${SRC_DIR}/backend/window/sdl/window_internal.cc
        ${SRC_DIR}/backend/window/sdl/vk/instance.h
        ${SRC_DIR}/backend/window/sdl/vk/instance.cc
        ${SRC_DIR}/backend/window/sdl/vk/window.h
        ${SRC_DIR}/backend/window/sdl/vk/window.cc
        ${SRC_DIR}/backend/window/sdl/gl/instance.h
        ${SRC_DIR}/backend/window/sdl/gl/window.h
)

target_link_directories(vk_engine PUBLIC
        ${SHADERC_LIBRARY_DIRS}
)

target_link_libraries(vk_engine
        Vulkan::Vulkan
        OpenGL::GL
        SDL2::SDL2
        SDL2::SDL2main
        GLEW::GLEW
        glfw
        ${SHADERC_LIBRARIES}
)