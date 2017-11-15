file(GLOB SOURCES "*.cpp")
add_executable(pie ${SOURCES})

# Components
target_link_libraries(pie piel)

# Externals
#
# Boost
find_package( Boost COMPONENTS program_options system filesystem log REQUIRED )
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(pie LINK_PUBLIC ${Boost_LIBRARIES})

target_link_libraries(pie crypto)

#install(TARGETS pie RUNTIME DESTINATION bin)
