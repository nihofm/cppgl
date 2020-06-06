project(CppGL)
cmake_minimum_required(VERSION 3.10)

if (NOT UNIX AND NOT WIN32)
	message(FATAL_ERROR "Unsupported platform!")
endif()

# ---------------------------------------------------------------------
# path management

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# ---------------------------------------------------------------------
# compiler options

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

if(UNIX)
    add_compile_options(-pthread -march=native)
    add_compile_options(-Wall -Wno-unused-parameter -Wno-unused-variable -Wno-int-to-pointer-cast)
else()
    add_definitions(-DNOMINMAX)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING) # wtf
endif()

include(GenerateExportHeader)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------------------------------------------------------------------
# traverse source tree

include_directories(src)
add_subdirectory(src)
add_subdirectory(examples)
