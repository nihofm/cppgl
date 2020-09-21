# glob for source files
file(GLOB SOURCES "*.cpp")

# executable target
add_executable(bbm-server ${SOURCES})

# output in source
set_target_properties(bbm-server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# handle dependencies
# Boost
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(bbm-server ${Boost_LIBRARIES})
# messages
target_link_libraries(bbm-server messages)
