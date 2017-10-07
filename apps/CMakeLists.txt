 
# Recurse into the "apps" subdirectory. This does not actually
# cause another cmake executable to run.
if(POLICY CMP0020)
cmake_policy(SET CMP0020 OLD)
endif()
set(CMAKE_AUTOMOC ON)
add_subdirectory (spiffy)
