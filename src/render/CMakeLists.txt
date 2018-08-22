cmake_minimum_required (VERSION 2.6)

file(GLOB SRC
    "${PROJECT_SOURCE_DIR}/include/render/*.h"
    "*.cpp"
)

add_library(engine_render STATIC ${SRC})
target_include_directories(engine_render
    PUBLIC "${PROJECT_SOURCE_DIR}/include/render"
    PUBLIC "${PROJECT_SOURCE_DIR}/external/stb"
    PUBLIC "${PROJECT_SOURCE_DIR}/external/tinyobjloader"
)
target_link_libraries(engine_render
    PUBLIC engine_graphics
)
