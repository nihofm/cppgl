# glob for source files
file(GLOB_RECURSE SOURCES "*.cpp")

# library target
add_library(imgui SHARED ${SOURCES})

# output in source
set_target_properties(imgui PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
