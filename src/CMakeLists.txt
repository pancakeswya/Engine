
add_subdirectory(backend)
add_subdirectory(engine)
add_subdirectory(obj)

add_executable(engine_main main.cc)

target_link_libraries(engine_main PUBLIC engine)