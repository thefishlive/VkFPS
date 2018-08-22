cmake_minimum_required (VERSION 2.6)

find_package(Vulkan REQUIRED)

file(GLOB SRC
    "${PROJECT_SOURCE_DIR}/include/graphics/*.h"
    "*.cpp"
)

add_library(engine_graphics STATIC ${SRC})
target_include_directories(engine_graphics
    PUBLIC "${PROJECT_SOURCE_DIR}/external/glfw/include"
    PUBLIC "${PROJECT_SOURCE_DIR}/external/glm"
    PUBLIC "${PROJECT_SOURCE_DIR}/external/vma/src"
    PUBLIC "${PROJECT_SOURCE_DIR}/include/graphics"
)
target_link_libraries(engine_graphics
    PUBLIC engine_utils
    PUBLIC Vulkan::Vulkan
    PUBLIC glfw
)
