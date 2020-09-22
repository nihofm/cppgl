# glob for source files
file(GLOB SOURCES "*.cpp")

set(TARGET bbm-a2)

# executable target
add_executable(${TARGET} ${SOURCES})

# output in source
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# define base path for mesh/image lookups
add_definitions(-DEXECUTABLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# handle dependencies

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(${TARGET} ${OPENGL_LIBRARIES})

set(DEPDIR "${CMAKE_CURRENT_SOURCE_DIR}/../common/thirdparty")

if(UNIX)
    find_library(GLEW GLEW PATHS ${DEPDIR}/lin/lib NO_DEFAULT_PATH)
	target_link_libraries(${TARGET} ${GLEW})
else()
	find_library(GLEW glew32 PATHS ${DEPDIR}/win/lib NO_DEFAULT_PATH)
	target_link_libraries(${TARGET} ${GLEW})
endif()

# common libs
target_link_libraries(${TARGET} imgui cppgl messages)
