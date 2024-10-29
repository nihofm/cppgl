# glob for source files
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_library(cppgl STATIC ${SOURCES})

# public interface (cppgl.h is an available header file for all dependant projects)
target_include_directories(cppgl PUBLIC .)
target_include_directories(cppgl PUBLIC ../submodules/glm)
target_include_directories(cppgl PRIVATE ../submodules/glew-cmake/include)
target_include_directories(cppgl PRIVATE ../submodules/glfw/include)
target_include_directories(cppgl PRIVATE ../submodules/assimp/include)

# ----------------------------------------------------------
# dependencies

# opengl
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_include_directories(cppgl PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(cppgl ${OPENGL_LIBRARIES})

if(UNIX)
    target_link_libraries(cppgl stdc++fs) # required for std::filesystem
endif()

target_link_libraries(cppgl libglew_static glfw assimp)
