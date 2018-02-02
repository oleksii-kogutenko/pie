set(Module_name piel)
file(GLOB SOURCES "*.cpp" "*.h"  "*.hpp" "logger/*.cpp" "logger/*.h" "logger/app/*.cpp" "logger/app/*.h" "logger/out/*.cpp" "logger/out/*.h" "logger/dispatcher/*.cpp" "logger/dispatcher/*.h")
add_library(${Module_name} STATIC ${SOURCES})
