cmake_minimum_required (VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(NAME VkFPS)
project (${NAME})

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

#
# External dependencies
#

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/build/")

add_subdirectory("external/glfw")
add_subdirectory("resources")
add_subdirectory("tools")

find_package(Vulkan REQUIRED)

#
# Project tuneables
#

option(ENABLE_VK_LAYER_VALIDATION "Enable VK_LAYER_LUNARG_standard_validation layers" OFF)
option(ENABLE_VK_LAYER_VALIDATION_DEBUG "Enable VK_LAYER_LUNARG_standard_validation debug info" OFF)

option(ENABLE_VK_LAYER_API_DUMP "Enable VK_LAYER_LUNARG_api_dump layer" OFF)
option(ENABLE_VK_LAYER_ASSISTANT "Enable VK_LAYER_LUNARG_assistant_layer layer" OFF)
option(ENABLE_VK_LAYER_MONITOR "Enable VK_LAYER_LUNARG_monitor layer" OFF)

option(ENABLE_DEBUG_LOGGING "Enable debug logging of messages" OFF)
option(ENABLE_DEBUG_ASSERT "Enable debug asserts" OFF)

set (SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

#
# Source file macros
#

macro(add_src file)
	list(APPEND SOURCE "${SRC_DIR}/${file}.cpp")

	if (${file} MATCHES "^r_")
		list(APPEND RENDER_SOURCE "${SRC_DIR}/${file}.cpp")
	elseif (${file} MATCHES "^g_")
		list(APPEND GRAPHICS_SOURCE "${SRC_DIR}/${file}.cpp")
	elseif (${file} MATCHES "^u_")
		list(APPEND UTILS_SOURCE "${SRC_DIR}/${file}.cpp")
	else()
		message(ERROR " File ${file} is not a match for any soruce group")
	endif()
endmacro()

macro(add_hdr file)
	list(APPEND HEADERS "${INCLUDE_DIR}/${file}.h")

	if (${file} MATCHES "^r_")
		list(APPEND RENDER_HEADERS "${INCLUDE_DIR}/${file}.h")
	elseif (${file} MATCHES "^g_")
		list(APPEND GRAPHICS_HEADERS "${INCLUDE_DIR}/${file}.h")
	elseif (${file} MATCHES "^u_")
		list(APPEND UTILS_HEADERS "${INCLUDE_DIR}/${file}.h")
	else()
		message(ERROR " File ${file} is not a match for any soruce group")
	endif()
endmacro()

macro(add_all file)
	add_src(${file})
	add_hdr(${file})
endmacro()

#
# Project source files
#

add_all("vulkan_fps")

add_all("g_device")
add_all("g_devmem")
add_all("g_fence")
add_all("g_pipeline")
add_all("g_pipeline_builder")
add_all("g_pipeline_cache")
add_all("g_queue")
add_all("g_renderpass")
add_all("g_swapchain")
add_all("g_shader")
add_all("g_transfer_context")
add_all("g_window")

add_all("r_camera")
add_all("r_model")
add_all("r_model_loader")
add_all("r_material")
add_all("r_scene")
add_all("r_renderer")

add_all("u_debug")
add_all("u_io")

add_hdr("r_shaderif")

add_hdr("u_defines")

set (GENERATED_SOURCE
    "${PROJECT_BINARY_DIR}/src/vk_ext.cpp"
)

set (GENERATED_HEADERS
    "${PROJECT_BINARY_DIR}/include/vk_ext.h"
)

#
# Target setup
#

add_executable(VulkanFps
    "${SOURCE}"
)
add_dependencies(VulkanFps CreateExtensionFile Resources)
target_link_libraries(VulkanFps
    PUBLIC Vulkan::Vulkan
    PUBLIC glfw
)
target_include_directories(VulkanFps
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glm"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/vma/src"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include"
)
target_sources(VulkanFps
    PRIVATE ${SHADERS}
    PRIVATE ${SHADERS_BIN}
    PRIVATE ${HEADERS}
    PRIVATE ${GENERATED_SOURCE}
)

source_group("Render\\Source Files" FILES ${RENDER_SOURCE})
source_group("Graphics\\Source Files" FILES ${GRAPHICS_SOURCE})
source_group("Utils\\Source Files" FILES ${UTILS_SOURCE})
source_group("Render\\Header Files" FILES ${RENDER_HEADERS})
source_group("Graphics\\Header Files" FILES ${GRAPHICS_HEADERS})
source_group("Utils\\Header Files" FILES ${UTILS_HEADERS})

if (WIN32)
set_target_properties(VulkanFps PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif(WIN32)

macro(compile_define d)
IF (${d})
	message(STATUS "Adding c define ${d}")
	target_compile_definitions(VulkanFps
		PRIVATE ${d}
	)
ENDIF ()
endmacro()

#
# Vulkan layer defines
#

compile_define(ENABLE_VK_LAYER_VALIDATION)
compile_define(ENABLE_VK_LAYER_VALIDATION_DEBUG)
compile_define(ENABLE_VK_LAYER_API_DUMP)
compile_define(ENABLE_VK_LAYER_ASSISTANT)
compile_define(ENABLE_VK_LAYER_MONITOR)

#
# Debug defines
#

compile_define(ENABLE_DEBUG_LOGGING)
compile_define(ENABLE_DEBUG_ASSERT)
