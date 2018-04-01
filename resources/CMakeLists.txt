cmake_minimum_required (VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(resource_shader shader out_shader out_hdr prefix)
add_custom_command(OUTPUT ${out_shader}
    COMMAND ${SPIRV_COMPILER} ${shader} -o ${out_shader}
    DEPENDS ${shader}
)
add_custom_command(OUTPUT ${out_hdr}
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/spv-to-header.py ${out_shader} ${out_hdr} ${prefix}
    DEPENDS ${out_shader}
)
endmacro()

macro(resource resource)
add_custom_command(OUTPUT ${resource}
    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/resources/${resource}
                    ${CMAKE_BINARY_DIR}/resources/${resource}
    DEPENDS ${CMAKE_SOURCE_DIR}/resources/${resource}
)
endmacro()

resource(models/sphere.obj)
resource(models/sphere.mtl)

set (RESOURCES
    "models/sphere.obj"
    "models/sphere.mtl"
)

add_custom_target(Resources
    DEPENDS ${RESOURCES}
)
