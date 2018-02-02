set(Module_name pie)

file(GLOB SOURCES "*.cpp")
add_executable(${Module_name} ${SOURCES})

# Components
target_link_libraries(${Module_name} piel commands pthread)

# Externals
#
# Boost (libboost-all-dev)
find_package( Boost COMPONENTS program_options system filesystem log regex REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${Module_name} LINK_PUBLIC ${Boost_LIBRARIES})

# OpenSSL (libssl-dev)
target_link_libraries(${Module_name} crypto)

# libzip (libzip-dev)
target_link_libraries(${Module_name} ${ZIP})

# libzip (libcurl4-openssl-dev)
target_link_libraries(${Module_name} curl)

#install(TARGETS pie RUNTIME DESTINATION bin)
