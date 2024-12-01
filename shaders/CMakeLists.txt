set(SHADER_SOURCE_DIR .)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS
        ${SHADER_SOURCE_DIR}/*.glsl
        ${SHADER_SOURCE_DIR}/*.vert
        ${SHADER_SOURCE_DIR}/*.frag
        ${SHADER_SOURCE_DIR}/*.comp
        ${SHADER_SOURCE_DIR}/*.geom
        ${SHADER_SOURCE_DIR}/*.tesc
        ${SHADER_SOURCE_DIR}/*.tese
        ${SHADER_SOURCE_DIR}/*.mesh
        ${SHADER_SOURCE_DIR}/*.task
        ${SHADER_SOURCE_DIR}/*.rgen
        ${SHADER_SOURCE_DIR}/*.rchit
        ${SHADER_SOURCE_DIR}/*.rmiss
)

add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
        OUTPUT ${SHADER_BINARY_DIR}
        COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADER_SOURCES)
    get_filename_component(FILENAME ${source} NAME)
    get_filename_component(SHADERNAME ${source} LAST_EXT)
    string(SUBSTRING "${SHADERNAME}" 1 -1 SHADERNAME)
    add_custom_command(
            COMMAND
            ${glslc_executable} -DMAX_TEXTURES=${SHADER_MAX_TEXTURE}
            -O -MD -MF ${SHADERNAME}.d
            -o ${SHADER_BINARY_DIR}/${SHADERNAME}.spv ${source}
            OUTPUT ${SHADER_BINARY_DIR}/${SHADERNAME}.spv
            DEPENDS ${FILENAME} ${SHADER_BINARY_DIR}
            DEPFILE ${SHADERNAME}.d
            COMMENT "Building shader ${SHADERNAME}.spv"
    )
    list(APPEND SHADER_SPV ${SHADER_BINARY_DIR}/${SHADERNAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SHADER_SPV})