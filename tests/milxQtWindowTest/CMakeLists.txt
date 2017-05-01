cmake_minimum_required(VERSION 2.6)
 
PROJECT(milxQtWindowTest)
 
INCLUDE_DIRECTORIES(${SMILI_SOURCE_DIR}/include/Qt ${SMILI_SOURCE_DIR}/include/vtk-ext ${SMILI_SOURCE_DIR}/include/itkVTKGlue ${MILXQT_INCLUDE_DIRS})
link_directories(${SMILI_BINARY_DIR}/lib)
 
ADD_EXECUTABLE(milxQtWindowTest milxQtWindowTest.cxx)
TARGET_LINK_LIBRARIES(milxQtWindowTest milx-Qt ${QT_LIBRARIES} ${ITK_LIBRARIES} vtk-ext ${VTK_LIBRARIES})
