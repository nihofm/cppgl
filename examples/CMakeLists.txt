# target name to generate here
set(TARGET example)

# glob source files
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

# define target
add_executable(${TARGET} ${SOURCES} ${HEADERS})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# install
install(TARGETS ${TARGET} DESTINATION bin)

# ----------------------------------------------------------
# dependencies

# std::filesystem
target_link_libraries(${TARGET} stdc++fs)

# built libs
target_link_libraries(${TARGET} cppgl)
