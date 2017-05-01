 
# Recurse into the "apps" subdirectory. This does not actually
# cause another cmake executable to run.
add_subdirectory (milxFileTest)
IF(BUILD_MILXQT)
  set(CMAKE_AUTOMOC ON)
  add_subdirectory (milxQtWindowTest)

ENDIF(BUILD_MILXQT)