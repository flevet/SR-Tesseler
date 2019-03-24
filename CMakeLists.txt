project(Tesseler)
cmake_minimum_required(VERSION 2.8)


#set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#if(WIN32)
#  set(CMAKE_WIN32_EXECUTABLE ON)
#endif()

# This code sorts the project files as they appear in the root directory

# Generate a list of all .c & .h files in the current directory and sub directores.

set(QCUSTOMPLOT_HEADER_FILES
  src/qcustomplot/QCPBarsWithColors.h
  src/qcustomplot/QCPHistogram.h
  src/qcustomplot/QCPStringLegendItem.h
  src/qcustomplot/qcustomplot.h
  )

set(QCUSTOMPLOT_SOURCE_FILES
  src/qcustomplot/QCPBarsWithColors.cpp
  src/qcustomplot/QCPHistogram.cpp
  src/qcustomplot/QCPStringLegendItem.cpp
  src/qcustomplot/qcustomplot.cpp
  )

set(HEADER_FILES
src/lmstruct.h
src/gl2ps.h
src/resource.h
src/lmmin.h
src/lmcurve.h
src/LoaderDetectionSet.hpp
src/KRipley.hpp
src/MainFilterDialog.hpp
src/ImageViewer.hpp
src/MiscFilterWidget.hpp
src/HistogramCamera.hpp
src/MiscQuantificationWidget.hpp
src/Histogram.hpp
src/MoleculeInfos.hpp
src/nanoflann.hpp
src/Geometry.hpp
src/NeuronObject.hpp
src/ObjectInterface.hpp
src/GeneralTools.hpp
src/Palette.hpp
src/FilterObjectWidget.hpp
src/DetectionSet.hpp
src/DetectionCleanerWidget.hpp
src/Roi.hpp
src/DetectionCleanerGLViewer.hpp
src/RoiManagerWidget.hpp
src/DetectionCleaner.hpp
src/SuperResObject.hpp
src/Vec2.hpp
src/Vec3.hpp
src/Vec4.hpp
src/DBScan.hpp
src/VoronoiObject.hpp
src/VoronoiWidget.hpp
src/Camera2D.hpp
src/WrapperVoronoiDiagram.hpp
)

set(SOURCE_FILES
src/gl2ps.c
src/lmcurve.c
src/lmmin.c
src/VoronoiObject.cpp
src/DetectionCleaner.cpp
src/SuperResObject.cpp
src/DetectionCleanerGLViewer.cpp
src/RoiManagerWidget.cpp
src/DetectionCleanerWidget.cpp
src/Roi.cpp
src/DetectionSet.cpp
src/main.cpp
src/DBScan.cpp
src/VoronoiWidget.cpp
src/GeneralTools.cpp
src/Palette.cpp
src/Geometry.cpp
src/NeuronObject.cpp
src/LoaderDetectionSet.cpp
src/Histogram.cpp
src/MoleculeInfos.cpp
src/HistogramCamera.cpp
src/MiscQuantificationWidget.cpp
src/WrapperVoronoiDiagram.cpp
src/ImageViewer.cpp
src/MiscFilterWidget.cpp
src/KRipley.cpp
src/MainFilterDialog.cpp
src/Camera2D.cpp
src/FilterObjectWidget.cpp
)

set(RESOURCE_FILES
  src/Resources/Icon.ico
  src/Resources/icon1.ico
  src/Resources/resource.h
  src/Resources/Tesseler.rc
  )

set(FOO_MOC_HEADERS
src/Camera2D.hpp
src/DetectionCleanerWidget.hpp
src/FilterObjectWidget.hpp
src/HistogramCamera.hpp
src/ImageViewer.hpp
src/MiscFilterWidget.hpp
src/MiscQuantificationWidget.hpp
src/RoiManagerWidget.hpp
src/VoronoiWidget.hpp
src/qcustomplot/QCPHistogram.h
src/qcustomplot/QCPStringLegendItem.h
src/qcustomplot/qcustomplot.h
)

set(FOO_SOURCE_FILES
src/ChoseFileDialogTreeModel.cpp
src/Camera2D.cpp
src/DetectionCleanerWidget.cpp
src/FilterObjectWidget.cpp
src/HistogramCamera.cpp
src/ImageViewer.cpp
src/MiscFilterWidget.cpp
src/MiscQuantificationWidget.cpp
src/RoiManagerWidget.cpp
src/VoronoiWidget.cpp
src/qcustomplot/QCPHistogram.cpp
src/qcustomplot/QCPStringLegendItem.cpp
src/qcustomplot/qcustomplot.cpp
)


set(header_list
  ${QCUSTOMPLOT_HEADER_FILES}
  ${HEADER_FILES}
  )

message("Headers: ${RESOURCE_FILES}")

set(source_list
  ${QCUSTOMPLOT_SOURCE_FILES}
  ${SOURCE_FILES}
  )

set(main_CPP
  src/main.cpp
  )

foreach(source IN LISTS header_list)
    get_filename_component(source_path "${source}" PATH)
    #message(STATUS ${source_path})
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    set(source_path_msvc "${source_path_msvc}\\Header Files")
    #message(STATUS ${source_path_msvc})
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
foreach(source IN LISTS source_list)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    set(source_path_msvc "${source_path_msvc}\\Source Files")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
foreach(source IN LISTS other_list)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
foreach(source IN LISTS RESOURCE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
get_filename_component(main_path "${main_CPP}" PATH)
string(REPLACE "/" "\\" main_path_msvc "${main_path}")
source_group("${main_path_msvc}" FILES "${main_CPP}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
"${CMAKE_SOURCE_DIR}/cmake/Modules/")
add_definitions(-DNOMINMAX)
find_package(CUDA QUIET)

find_package(CGAL REQUIRED COMPONENTS Core)
if(CGAL_FOUND)
    message("Found CGAL.")
    include(${CGAL_USE_FILE})

    message("CGAL_CXX_FLAGS_INIT: ${CGAL_CXX_FLAGS_INIT}")
    add_definitions(${CGAL_CXX_GLAFS_INIT})

    message("CGAL_LIBRARIES: ${CGAL_LIBRARY_DIRS}")
    message("CGAL_3RD_PARTY_LIBRARIES: ${CGAL_3RD_PARTY_LIBRARIES_DIRS}")
    #target_link_libraries(Tesseler ${CGAL_LIBRARY_DIRS} ${CGAL_3RD_PARTY_LIBRARIES_DIRS})
else()
    message(FATAL_ERROR "CGAL is required.")
endif()

find_package(Qt4 4.8 REQUIRED QtGui QtCore QtOpenGL)
qt4_wrap_cpp (FOO_MOC_OUTFILES ${FOO_MOC_HEADERS})
message("MOC: ${FOO_MOC_OUTFILES}")

add_executable(Tesseler ${header_list} ${source_list} ${other_list} ${RESOURCE_FILES} ${main_CPP} ${FOO_MOC_OUTFILES})

target_link_libraries(Tesseler Qt4::QtGui Qt4::QtCore Qt4::QtOpenGL)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(Tesseler ${OPENGL_LIBRARIES})
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(Tesseler ${GLEW_LIBRARIES})