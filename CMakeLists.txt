project (Aort)
# set cmake minimum cmake version
cmake_minimum_required (VERSION 2.8)
# select relwithdebinfo build type
set (CMAKE_BUILD_TYPE RelWithDebInfo)

find_package (Qt4 REQUIRED)

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})

include_directories (${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)
# enable openmp support
set (CMAKE_CXX_FLAGS "-fopenmp")
# add sources
set (SOURCES
  src/AortLight.cpp
  src/AortMaterial.cpp
  src/AortMeshParser.cpp
  src/AortRenderer.cpp
  src/AortSceneNode.cpp
  src/AortTexture.cpp
  src/AortTriangle.cpp
  src/LanguageManager.cpp
  src/Main.cpp
  src/MainWindow.cpp
  src/OgreManager.cpp
  src/OgreWidget.cpp
 )
# add ui files
qt4_wrap_ui (UI_SOURCES
  src/MainWindow.ui
)
# add moc headers
qt4_wrap_cpp (MOC_SOURCES
  src/LanguageManager.h
  src/MainWindow.h
  src/OgreManager.h
  src/OgreWidget.h
)
# add resources
qt4_add_resources (RESOURCES
  resources.qrc
)
# add executable
add_executable (Aort WIN32 ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES} ${RESOURCES} resources.rc)
target_link_libraries (Aort ${QT_LIBRARIES} OgreMain assimp)
