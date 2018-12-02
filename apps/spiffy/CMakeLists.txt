cmake_minimum_required(VERSION 2.6)
 if(POLICY CMP0020)
cmake_policy(SET CMP0020 OLD)
endif()
PROJECT(spiffy)
find_package(Qt5Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

QT5_WRAP_UI(Cmake_form_hdr ${SMILI_SOURCE_DIR}/ui/spiffy.ui)

INCLUDE_DIRECTORIES(${SMILI_SOURCE_DIR}/include/Qt ${SMILI_SOURCE_DIR}/include/vtk-ext ${SMILI_SOURCE_DIR}/include/itkVTKGlue ${MILXQT_INCLUDE_DIRS})
link_directories(${SMILI_BINARY_DIR}/lib)

set(SPIFFY_ICON_FILE ${PROJECT_SOURCE_DIR}/spiffy.rc) 
 
ADD_EXECUTABLE(spiffy MACOSX_BUNDLE ${SPIFFY_ICON_FILE} spiffy.cxx ${Cmake_form_hdr})
TARGET_LINK_LIBRARIES(spiffy milx-Qt ${QT_LIBRARIES} ${ITK_LIBRARIES} vtk-ext ${VTK_LIBRARIES})


#Packaging
SET(CPACK_PACKAGE_NAME "SPIFFY")

# for windows, use nsis
SET(CPACK_GENERATOR NSIS)


INCLUDE(CPack)
