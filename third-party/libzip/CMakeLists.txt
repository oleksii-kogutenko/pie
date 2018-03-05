#add_library(zip_cust SHARED IMPORTED)
#set_target_properties(zip_cust PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libzip.so")

find_library(ZIP NAMES zip PATHS "${CMAKE_SOURCE_DIR}/third-party/libzip/lib" NO_DEFAULT_PATH)
message("ZIP: ${ZIP}")
include_directories("${CMAKE_SOURCE_DIR}/third-party/libzip/include")

#include_directories("include")
