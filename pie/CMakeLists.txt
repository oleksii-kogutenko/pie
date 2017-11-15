file(GLOB SOURCES "*.cpp")
add_executable(pie ${SOURCES})

target_link_libraries(pie "piel")
target_link_libraries(pie crypto)
target_link_libraries(pie boost_system boost_filesystem)

#install(TARGETS pie RUNTIME DESTINATION bin)
