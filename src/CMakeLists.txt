# target name to generate here
set(TARGET cppgl)

# glob source files
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_library(${TARGET} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${TARGET} PROPERTIES VERSION 0.1)

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

	# glew
	find_package(GLEW REQUIRED)
	target_include_directories(${TARGET} PUBLIC ${GLEW_INCLUDE_DIR})
	target_link_libraries(${TARGET} ${GLEW_LIBRARIES})

	# glfw3
	find_package(glfw3 REQUIRED)
	target_link_libraries(${TARGET} glfw)

	# ass-imp
	find_package(ASSIMP REQUIRED)
	include_directories(${ASSIMP_INCLUDE_DIRS})
	target_link_libraries(cppgl ${ASSIMP_LIBRARIES})

	# std::filesystem
	target_link_libraries(${TARGET} stdc++fs)
else()
	find_library(ASSIMP assimp PATHS ${DEPDIR}/lib NO_DEFAULT_PATH)
	find_library(GLEW glew32 PATHS ${DEPDIR}/lib)
    find_library(GLFW glfw3 PATHS ${DEPDIR}/lib)

    target_link_libraries(${TARGET} ${GLEW} ${GLFW} ${ASSIMP})
	include_directories(${TARGET} ${DEPDIR}/include)

endif()