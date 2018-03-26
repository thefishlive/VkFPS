cmake_minimum_required (VERSION 2.6)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(PythonInterp 3 REQUIRED)

##########################
# VK Extension Generator #
##########################
set (VK_EXT_GEN_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/vk-ext-generator.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/cgen.py"
)
set(VK_EXT_GEN_SRC_OUT_DIR
    "${PROJECT_BINARY_DIR}/src"
)
set(VK_EXT_GEN_HDR_OUT_DIR
    "${PROJECT_BINARY_DIR}/include"
)
set (VK_EXT_GEN_EXTENSIONS
    "VK_EXT_debug_marker VK_EXT_debug_report"
    CACHE STRING "Extensions to generate for extension file"
)
add_custom_target(CreateExtensionFile
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VK_EXT_GEN_HDR_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VK_EXT_GEN_SRC_OUT_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/vk-ext-generator.py ${VK_EXT_GEN_HDR_OUT_DIR}/vk_ext.h ${VK_EXT_GEN_SRC_OUT_DIR}/vk_ext.cpp ${PROJECT_SOURCE_DIR}/tools/data/vk.xml VK_EXT_debug_marker VK_EXT_debug_report
    SOURCES ${VK_EXT_GEN_SRC}
)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VK_EXT_GEN_HDR_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VK_EXT_GEN_SRC_OUT_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/vk-ext-generator.py ${VK_EXT_GEN_HDR_OUT_DIR}/vk_ext.h ${VK_EXT_GEN_SRC_OUT_DIR}/vk_ext.cpp ${PROJECT_SOURCE_DIR}/tools/data/vk.xml VK_EXT_debug_marker VK_EXT_debug_report
)
source_group("Python Source" FILES
	${VK_EXT_GEN_SRC}
)
