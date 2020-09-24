# glob for source files
file(GLOB SOURCES "*.cpp")

set(TARGET pacmap-a4)

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

set(DEPDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../common/thirdparty")

if(UNIX)
    #find_library(GLEW GLEW PATHS ${DEPDIR}/lin/lib NO_DEFAULT_PATH)
	#target_link_libraries(${TARGET} ${GLEW})

    #linux: preinstalled expat
    target_link_libraries(${TARGET} expat)
else()
    target_include_directories(${TARGET} PUBLIC ${DEPDIR}/include)

    #windows: extern expat
    target_include_directories(${TARGET} PUBLIC ${DEPDIR}/include/expat)
    find_library(EXPAT_D libexpatd PATHS ${DEPDIR}/win/lib NO_DEFAULT_PATH)
    find_library(EXPAT libexpat PATHS ${DEPDIR}/win/lib NO_DEFAULT_PATH)
    target_link_libraries(${TARGET} optimized ${EXPAT} debug ${EXPAT_D})

    # post build dll copy
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../common/cppgl/external/thirdparty/win/bin/x64" $<TARGET_FILE_DIR:${TARGET}>)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:cppgl> $<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_NAME:cppgl>)
endif()
# common libs
target_link_libraries(${TARGET} cppgl)
# parser lib
