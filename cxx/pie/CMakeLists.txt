file(GLOB SOURCES "*.cpp")
add_executable(pie ${SOURCES})

target_link_libraries(gpie crypto)
target_link_libraries(gpie boost_system boost_filesystem)

#install(TARGETS pie RUNTIME DESTINATION bin)
