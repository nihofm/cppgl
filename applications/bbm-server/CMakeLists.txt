# glob for source files
file(GLOB SOURCES "*.cpp")

set(TARGET bbm-server)

# executable target
add_executable(${TARGET} ${SOURCES})

# output in source
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# handle dependencies
# Boost
if(UNIX)
    find_package(Boost REQUIRED COMPONENTS system)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${TARGET} ${Boost_LIBRARIES})
else()

    find_library(BOOST Boost PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../boost NO_DEFAULT_PATH)
  #  include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${TARGET} BOOST)
endif()
# messages
target_link_libraries(${TARGET} messages)
