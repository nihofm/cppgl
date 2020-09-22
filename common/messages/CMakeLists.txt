# glob for source files
file(GLOB SOURCES "*.cpp")

# library target
add_library(messages STATIC ${SOURCES})

# set version and output in source
set_target_properties(messages PROPERTIES VERSION 0.1)
set_target_properties(messages PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

include(ExternalProject)

# handle dependencies
if(UNIX)
    # Boost
    find_package(Boost REQUIRED COMPONENTS system)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(messages ${Boost_LIBRARIES})
else()
    #find_library(BOOST Boost PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../boost NO_DEFAULT_PATH)
       # target_include_directories(${TARGET} ${LOCAL_BOOST_DIR})
    find_package(Boost REQUIRED COMPONENTS system)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(messages ${Boost_LIBRARIES})
    #target_link_libraries(messages ${BOOST})
endif()



