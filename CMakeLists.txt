cmake_minimum_required(VERSION 3.16)

PROJECT(CppGL
    VERSION 1.9
    DESCRIPTION "OpenGL framework for use in teaching and research with focus on simplicity and reusability."
    LANGUAGES CXX
)

if (NOT UNIX AND NOT WIN32)
	message(FATAL_ERROR "Unsupported platform!")
endif()

# ---------------------------------------------------------------------
# cmake options

option(CPPGL_BUILD_EXAMPLES "" OFF)

# ---------------------------------------------------------------------
# submodules

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)

set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(submodules/glew-cmake)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
add_subdirectory(submodules/glfw)

# assimp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(submodules/assimp)

# ---------------------------------------------------------------------
# compiler options

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

# ensure to use included deps
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_LINK_DIRECTORIES_BEFORE ON)

if(UNIX)
    add_compile_options(-pthread -march=native)
else()
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
endif()

# ---------------------------------------------------------------------
# traverse source tree

add_subdirectory(src)

if (CPPGL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
