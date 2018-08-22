cmake_minimum_required (VERSION 2.6)

file(GLOB SRC
    "${PROJECT_SOURCE_DIR}/include/logic/*.h"
    "*.cpp"
)

add_library(engine_logic STATIC ${SRC})
target_include_directories(engine_logic
    PUBLIC ${PROJECT_SOURCE_DIR}/include/logic
)
target_link_libraries(engine_logic
    PUBLIC engine_utils
)
