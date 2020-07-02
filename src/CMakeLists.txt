# target name to generate here
set(TARGET cppgl)

# glob source files
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_library(${TARGET} STATIC ${SOURCES} ${HEADERS})
set_target_properties(${TARGET} PROPERTIES VERSION 0.1 DESCRIPTION "xx")

# install
install(TARGETS ${TARGET} DESTINATION lib)
install(DIRECTORY . DESTINATION include/${TARGET} FILES_MATCHING PATTERN "*.h")

# ----------------------------------------------------------
# dependencies

# opengl
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_include_directories(${TARGET} PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(${TARGET} ${OPENGL_LIBRARIES})

if(UNIX)
    find_library(GLEW GLEW PATHS ${DEPDIR}/lin/lib NO_DEFAULT_PATH)
    find_library(GLFW glfw PATHS ${DEPDIR}/lin/lib NO_DEFAULT_PATH)
	find_library(ASSIMP assimp PATHS ${DEPDIR}/lin/lib NO_DEFAULT_PATH)
	target_link_libraries(${TARGET} stdc++fs) # required for std::filesystem
else()
	find_library(GLEW glew32 PATHS ${DEPDIR}/win/lib NO_DEFAULT_PATH)
    find_library(GLFW glfw3 PATHS ${DEPDIR}/win/lib NO_DEFAULT_PATH)
	find_library(ASSIMP assimp PATHS ${DEPDIR}/win/lib NO_DEFAULT_PATH)
	target_compile_definitions(${TARGET} PUBLIC -DNNG_STATIC_LIB)
endif()

target_link_libraries(${TARGET} ${GLEW} ${GLFW} ${ASSIMP})
