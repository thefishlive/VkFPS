cmake_minimum_required (VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(NAME VkFPS)
project (${NAME})

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/build/")

add_subdirectory("external/glfw")
add_subdirectory("resources")
add_subdirectory("tools")

find_package(Vulkan REQUIRED)

OPTION(ENABLE_VK_LAYER_VALIDATION "Enable VK_LAYER_LUNARG_standard_validation layers" OFF)
OPTION(ENABLE_VK_LAYER_VALIDATION_DEBUG "Enable VK_LAYER_LUNARG_standard_validation debug info" OFF)

OPTION(ENABLE_VK_LAYER_API_DUMP "Enable VK_LAYER_LUNARG_api_dump layer" OFF)
OPTION(ENABLE_VK_LAYER_ASSISTANT "Enable VK_LAYER_LUNARG_assistant_layer layer" OFF)
OPTION(ENABLE_VK_LAYER_MONITOR "Enable VK_LAYER_LUNARG_monitor layer" OFF)

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set (SOURCE
    "${SRC_DIR}/vulkan_fps.cpp"
	"${SRC_DIR}/g_device.cpp"
    "${SRC_DIR}/g_pipeline.cpp"
    "${SRC_DIR}/g_queue.cpp"
)

set (GENERATED_SOURCE
    "${PROJECT_BINARY_DIR}/src/vk_ext.cpp"
)

set (HEADERS
    "${INCLUDE_DIR}/vulkan_fps.h"
    "${INCLUDE_DIR}/g_device.h"
    "${INCLUDE_DIR}/g_pipeline.h"
    "${INCLUDE_DIR}/g_queue.h"
    "${INCLUDE_DIR}/vk_ext_internal.h"
)

set (GENERATED_HEADERS
    "${PROJECT_BINARY_DIR}/include/vk_ext.h"
)

add_executable(VulkanFps
    "${SOURCE}"
)
add_dependencies(VulkanFps CreateExtensionFile Resources)
target_link_libraries(VulkanFps
    PUBLIC Vulkan::Vulkan
    PUBLIC glfw
)
target_include_directories(VulkanFps
    PUBLIC external/glfw/include
    PUBLIC external/glm
    PUBLIC external/vma/src
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include"
)
target_sources(VulkanFps
    PRIVATE ${SHADERS}
    PRIVATE ${SHADERS_BIN}
    PRIVATE ${HEADERS}
    PRIVATE ${GENERATED_SOURCE}
)
IF (ENABLE_VK_LAYER_VALIDATION)
    target_compile_definitions(VulkanFps
        PRIVATE ENABLE_VK_LAYER_VALIDATION
    )
ENDIF (ENABLE_VK_LAYER_VALIDATION)

IF (ENABLE_VK_LAYER_VALIDATION_DEBUG)
    target_compile_definitions(VulkanFps
        PRIVATE ENABLE_VK_LAYER_VALIDATION_DEBUG
    )
ENDIF (ENABLE_VK_LAYER_VALIDATION_DEBUG)

IF (ENABLE_VK_LAYER_API_DUMP)
    target_compile_definitions(VulkanFps
        PRIVATE ENABLE_VK_LAYER_API_DUMP
    )
ENDIF (ENABLE_VK_LAYER_API_DUMP)

IF (ENABLE_VK_LAYER_ASSISTANT)
    target_compile_definitions(VulkanFps
        PRIVATE ENABLE_VK_LAYER_ASSISTANT
    )
ENDIF (ENABLE_VK_LAYER_ASSISTANT)

IF (ENABLE_VK_LAYER_MONITOR)
    target_compile_definitions(VulkanFps
        PRIVATE ENABLE_VK_LAYER_MONITOR
    )
ENDIF (ENABLE_VK_LAYER_MONITOR)
