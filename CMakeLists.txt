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
add_subdirectory("src")

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

file(GLOB SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

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
	PRIVATE engine_utils
	PRIVATE engine_graphics
	PRIVATE engine_render
	PRIVATE engine_logic
)
target_include_directories(VulkanFps
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include"
)
target_sources(VulkanFps
    PRIVATE ${SHADERS}
    PRIVATE ${SHADERS_BIN}
    PRIVATE ${HEADERS}
    PRIVATE ${GENERATED_SOURCE}
)

if (WIN32)
set_target_properties(VulkanFps PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif(WIN32)

macro(compile_define d)
IF (${d})
	message(STATUS "Adding c define ${d}")
	target_compile_definitions(engine_utils
		PRIVATE ${d}
	)
	target_compile_definitions(engine_graphics
		PRIVATE ${d}
	)
	target_compile_definitions(engine_render
		PRIVATE ${d}
	)
	target_compile_definitions(engine_logic
		PRIVATE ${d}
	)
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
