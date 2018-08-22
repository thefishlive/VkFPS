cmake_minimum_required (VERSION 2.6)

file(GLOB SRC
    "${PROJECT_SOURCE_DIR}/include/utils/*.h"
    "*.cpp"
)

add_library(engine_utils STATIC ${SRC})
target_include_directories(engine_utils
    PUBLIC ${PROJECT_SOURCE_DIR}/include/utils
)
