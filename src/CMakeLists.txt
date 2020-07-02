# target name to generate here
set(TARGET cppgl)

# glob source files
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

if(UNIX)
	# define target
	add_library(${TARGET} SHARED ${SOURCES} ${HEADERS})
	set_target_properties(${TARGET} PROPERTIES VERSION 0.1 DESCRIPTION "xx")
	generate_export_header(${TARGET})

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

	# for simplicity: Use static lib for Windows

	# define target
	add_library(${TARGET} STATIC ${SOURCES} ${HEADERS})
	set_target_properties(${TARGET} PROPERTIES VERSION 0.1 DESCRIPTION "xx")
	generate_export_header(${TARGET})

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

	find_library(ASSIMP assimp PATHS ${DEPDIR}/lib NO_DEFAULT_PATH)
	find_library(GLEW glew32 PATHS ${DEPDIR}/lib)
    find_library(GLFW glfw3 PATHS ${DEPDIR}/lib)
	target_compile_definitions(${TARGET} PUBLIC -DNNG_STATIC_LIB)

    target_link_libraries(${TARGET} ${GLEW} ${GLFW} ${ASSIMP})

endif()