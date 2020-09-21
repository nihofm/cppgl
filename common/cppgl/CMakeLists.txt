# glob for source files
file(GLOB_RECURSE SOURCES "*.cpp")

# library target
add_library(cppgl SHARED ${SOURCES})

# set version and output in source
set_target_properties(cppgl PROPERTIES VERSION 0.1)
set_target_properties(cppgl PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# handle dependencies
# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(cppgl ${GLEW_LIBRARIES})
# OpenGL
find_package(OpenGL REQUIRED)
add_definitions(-DOPENGL_INCLUDE_PATH="${OPENGL_INCLUDE_PATH}")
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(cppgl ${OPENGL_LIBRARIES})
# GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(cppgl glfw)
# Ass-Imp
find_package(ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS})
target_link_libraries(cppgl ${ASSIMP_LIBRARIES})
# imgui
target_link_libraries(cppgl imgui)
