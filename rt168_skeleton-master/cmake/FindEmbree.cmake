find_path(EMBREE_INCLUDE_PATH embree3/rtcore.h
  ${CMAKE_SOURCE_DIR}/embree/include)

if (APPLE)
  find_library(EMBREE_LIBRARY NAMES embree3 PATHS
    ${CMAKE_SOURCE_DIR}/embree/lib-macos NO_DEFAULT_PATH)
elseif (WIN32)
  find_library(EMBREE_LIBRARY NAMES embree3 PATHS
    ${CMAKE_SOURCE_DIR}/embree/lib-win32 NO_DEFAULT_PATH)
else ()
  find_library(EMBREE_LIBRARY NAMES embree3 PATHS
    ${CMAKE_SOURCE_DIR}/embree/lib-linux NO_DEFAULT_PATH)
endif ()

if (EMBREE_INCLUDE_PATH AND EMBREE_LIBRARY)
  set(EMBREE_FOUND TRUE)
endif ()