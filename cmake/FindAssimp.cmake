# Find the Asset Import Library (ASSIMP)
# This module defines the following variables:
#  ASSIMP_INCLUDE_DIRS - include directories for ASSIMP
#  ASSIMP_LIBRARIES - libraries to link against ASSIMP
#  ASSIMP_FOUND - true if ASSIMP has been found and can be used

find_path(ASSIMP_INCLUDE_DIR assimp/Importer.hpp
  HINTS ENV ASSIMP_DIR
  PATH_SUFFIXES include
)
find_library(ASSIMP_LIBRARY NAMES assimp
  HINTS ENV ASSIMP_DIR 
  PATH_SUFFIXES lib lib64 lib/assimp_debug-dll_win32 lib/assimp_release-dll_win32
)

set(ASSIMP_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
set(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Assimp REQUIRED_VARS ASSIMP_LIBRARY ASSIMP_INCLUDE_DIR)

mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)
