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

#set(DEPDIR "${CMAKE_CURRENT_SOURCE_DIR}/../common/thirdparty")

if(UNIX)
    #find_library(GLEW GLEW PATHS ${DEPDIR}/lin/lib NO_DEFAULT_PATH)
	#target_link_libraries(${TARGET} ${GLEW})

	# Boost
    find_package(Boost COMPONENTS program_options REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${TARGET} ${Boost_LIBRARIES})
else()
    # Boost
    find_package(Boost COMPONENTS program_options REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${TARGET} ${Boost_LIBRARIES})

    # post build dll copy
  	add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../common/cppgl/external/thirdparty/win/bin/x64" $<TARGET_FILE_DIR:${TARGET}>)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:cppgl> $<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_NAME:cppgl>)
endif()

# common libs
target_link_libraries(${TARGET} cppgl messages)


