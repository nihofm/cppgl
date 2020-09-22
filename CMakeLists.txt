cmake_minimum_required(VERSION 3.10)

if (NOT UNIX AND NOT WIN32)
	message(FATAL_ERROR "Unsupported platform!")
endif()

project("GraPa")

# set project name

# ---------------------------------------------------------------------
# compiler options

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

# setup for included dependencies
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_LINK_DIRECTORIES_BEFORE ON)


if(UNIX)
    add_compile_options(-pthread -march=native)
    add_compile_options(-Wall -pthread -pedantic -fpermissive -std=c++11 -Wno-int-to-pointer-cast)
	set(CMAKE_CXX_FLAGS_DEBUG          "-g")

    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
	
else()
    add_definitions(-DNOMINMAX)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING) # wtf
    #with cmake version >3.8, CMAKE_INTERPROCEDURAL_OPTIMIZATION ON breaks windows dll export
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


# GL preference
set(OpenGL_GL_PREFERENCE "GLVND")


#boost on windows
if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../boost/)
endif()



# additional global includes
include_directories(common)

# traverse
add_subdirectory(common)
add_subdirectory(applications)
