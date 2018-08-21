set(Module_name depman)

file(GLOB SOURCES "*.cpp")
add_executable(${Module_name} ${SOURCES})

# Externals
#
# Boost (libboost-all-dev)
find_package( Boost COMPONENTS ${Boost_libraries_list} REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${Module_name} LINK_PUBLIC ${Boost_LIBRARIES})

