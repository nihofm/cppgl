# glob for source files
file(GLOB SOURCES "*.cpp")

# executable target
add_executable(bbm-a1 ${SOURCES})

# output in source
set_target_properties(bbm-a1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# define base path for mesh/image lookups
add_definitions(-DEXECUTABLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# handle dependencies
# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(bbm-a1 ${GLEW_LIBRARIES})
# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(bbm-a1 ${OPENGL_LIBRARIES})
# common libs
target_link_libraries(bbm-a1 imgui cppgl messages)
