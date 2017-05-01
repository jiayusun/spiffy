cmake_minimum_required(VERSION 2.6)
 
PROJECT(milxQtRenderWindowTest)
 
INCLUDE_DIRECTORIES(${SMILI_SOURCE_DIR}/include/Qt ${SMILI_SOURCE_DIR}/include/vtk-ext ${SMILI_SOURCE_DIR}/include/itkVTKGlue ${MILXQT_INCLUDE_DIRS})
link_directories(${SMILI_BINARY_DIR}/lib)
 
ADD_EXECUTABLE(milxQtRenderWindowTest milxQtRenderWindowTest.cxx)
TARGET_LINK_LIBRARIES(milxQtRenderWindowTest milx-Qt ${QT_LIBRARIES} ${ITK_LIBRARIES} vtk-ext ${VTK_LIBRARIES})
