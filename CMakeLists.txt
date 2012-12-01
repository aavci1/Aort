CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(Aort)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)

ADD_DEFINITIONS(-Wall -O2 -g -ftree-vectorize -msse -msse2 -msse3 -mssse3 -fvariable-expansion-in-unroller)

# Qt
FIND_PACKAGE(Qt4 REQUIRED)
IF(QT_FOUND)
  SET(QT_USE_QTXML TRUE)
  INCLUDE(${QT_USE_FILE})
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)
ELSE()
  MESSAGE(FATAL_ERROR "Qt not found! Please make sure Qt is installed.")
ENDIF()

# OGRE
SET(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/cmake/;${CMAKE_MODULE_PATH}")

FIND_PACKAGE(OGRE REQUIRED)
IF(OGRE_FOUND)
  INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIR})
ELSE()
  MESSAGE(FATAL_ERROR "OGRE not found! Please make sure OGRE is installed and OGRE_HOME is set correctly.")
ENDIF()

# Boost
IF(WIN32)
  SET(OGRE_BOOST_COMPONENTS)
ELSE()
  SET(OGRE_BOOST_COMPONENTS system)
ENDIF()

FIND_PACKAGE(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
  ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ELSE()
  MESSAGE(FATAL_ERROR "Boost not found! Please make sure Boost is installed and BOOST_ROOT is set correctly.")
ENDIF()

# Assimp
FIND_PACKAGE(Assimp REQUIRED)
IF(ASSIMP_FOUND)
  INCLUDE_DIRECTORIES(${ASSIMP_INCLUDE_DIR})
ELSE()
  MESSAGE(FATAL_ERROR "Assimp not found! Please make sure Assimp is installed and ASSIMP_DIR is set correctly.")
ENDIF()

# Threads
FIND_PACKAGE(Threads REQUIRED)
IF(NOT Threads_FOUND)
  MESSAGE(FATAL_ERROR "Threads not found!")
ENDIF()

# OpenMP
FIND_PACKAGE(OpenMP)

IF(OPENMP_FOUND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ELSE()
  MESSAGE(STATUS "OpenMP not found!")
ENDIF()

# add sources
SET(SOURCES
  src/AortLight.cpp
  src/AortMaterial.cpp
  src/AortMeshParser.cpp
  src/AortRenderer.cpp
  src/AortSceneNode.cpp
  src/AortTexture.cpp
  src/AortTriangle.cpp
  src/Main.cpp
  src/MainWindow.cpp
  src/OgreManager.cpp
  src/OgreWidget.cpp
  src/TranslationManager.cpp
)
# add ui files
QT4_WRAP_UI(UI_SOURCES
  src/MainWindow.ui
)
# add headers
QT4_WRAP_CPP(MOC_SOURCES
  src/MainWindow.h
  src/OgreManager.h
  src/OgreWidget.h
  src/TranslationManager.h
)
# add resources
QT4_ADD_RESOURCES(RESOURCES
  resources.qrc
)

ADD_EXECUTABLE(Aort WIN32 ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES} ${RESOURCES} resources.rc)
TARGET_LINK_LIBRARIES(Aort ${QT_LIBRARIES} ${OGRE_LIBRARIES} ${Boost_LIBRARIES} ${ASSIMP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
