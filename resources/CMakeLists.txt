cmake_minimum_required (VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(RESOURCES "")
set(SOURCES "")
set(SHADERS_SOURCES "")
set(RESOURCES_SOURCES "")

#
# Resource type macros
#

if(WIN32)
set(SPIRV_COMPILER "glslc.exe")
else(WIN32)
set(SPIRV_COMPILER "glslc")
endif(WIN32)

macro(resource_shader shader prefix)
add_custom_command(OUTPUT ${shader}
	DEPENDS ${CMAKE_BINARY_DIR}/resources/${shader}.spv.h
)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/resources/${shader}.spv
    COMMAND ${SPIRV_COMPILER} ${CMAKE_SOURCE_DIR}/resources/${shader} -o ${CMAKE_BINARY_DIR}/resources/${shader}.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/resources/${shader}
)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/resources/${shader}.spv.h
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/spv-to-header.py ${CMAKE_BINARY_DIR}/resources/${shader}.spv ${CMAKE_BINARY_DIR}/resources/${shader}.spv.h ${prefix}
    DEPENDS ${CMAKE_BINARY_DIR}/resources/${shader}.spv
)
list(APPEND RESOURCES ${shader})
list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/resources/${shader})
list(APPEND SHADERS_SOURCES ${CMAKE_SOURCE_DIR}/resources/${shader})
endmacro()

macro(resource resource)
add_custom_command(OUTPUT ${resource}
    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/resources/${resource}
                    ${CMAKE_BINARY_DIR}/resources/${resource}
    DEPENDS ${CMAKE_SOURCE_DIR}/resources/${resource}
)
list(APPEND RESOURCES ${resource})
list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/resources/${resource})
list(APPEND RESOURCES_SOURCES ${CMAKE_SOURCE_DIR}/resources/${resource})
endmacro()

#
# Resources to include in build
#

resource(models/sphere.obj)
resource(models/sphere.mtl)
resource(models/plane.obj)
resource(models/plane.mtl)
resource(models/bmps_27.obj)
resource(models/bmps_27.mtl)
resource(models/scene-Helicopter-27.obj)
resource(models/scene-Helicopter-27.mtl)

resource_shader(shaders/model.vert model_vert)
resource_shader(shaders/model.frag model_frag)
resource_shader(shaders/standard.vert standard_vert)
resource_shader(shaders/standard.frag standard_frag)
resource_shader(shaders/deferred.vert deffered_vert)
resource_shader(shaders/deferred.frag deffered_frag)

add_custom_target(Resources
    DEPENDS ${RESOURCES}
    SOURCES ${SOURCES}
)

source_group(Shaders FILES ${SHADERS_SOURCES})
source_group(Resources FILES ${RESOURCES_SOURCES})
