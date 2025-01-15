
macro(make_shaders NAME_IN NAME_OUT)
    file(GLOB SHADERS shaders/*)
    foreach(SHADER ${SHADERS})
        get_filename_component(SHADERNAME ${SHADER} NAME)
        file(READ ${SHADER} ${SHADERNAME})
    endforeach ()
    configure_file(${NAME_IN} ${NAME_OUT} @ONLY)
endmacro()