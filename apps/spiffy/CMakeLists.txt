cmake_minimum_required(VERSION 2.6)
 
PROJECT(spiffy)
find_package(Qt5Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

QT5_WRAP_UI(Cmake_form_hdr ${SMILI_SOURCE_DIR}/ui/spiffy.ui)

INCLUDE_DIRECTORIES(${SMILI_SOURCE_DIR}/include/Qt ${SMILI_SOURCE_DIR}/include/vtk-ext ${SMILI_SOURCE_DIR}/include/itkVTKGlue ${MILXQT_INCLUDE_DIRS})
link_directories(${SMILI_BINARY_DIR}/lib)
 
ADD_EXECUTABLE(spiffy spiffy.cxx ${Cmake_form_hdr})
TARGET_LINK_LIBRARIES(spiffy milx-Qt ${QT_LIBRARIES} ${ITK_LIBRARIES} vtk-ext ${VTK_LIBRARIES})
