set(Module_name commands)

file(GLOB SOURCES "*.cpp" "*.h"  "*.hpp")
add_library(${Module_name} STATIC ${SOURCES})

target_link_libraries(${Module_name} piel)
