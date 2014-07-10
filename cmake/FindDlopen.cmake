MESSAGE(STATUS "Detecting dlopen")

FIND_PATH(DLOPEN_INCLUDE_DIR dlfcn.h
  PATHS /usr/local/include /usr/include /sw/include /opt/local/include /opt/include )

FIND_LIBRARY(DLOPEN_LIBRARY NAMES dl
  PATH_SUFFIXES lib64 lib
  PATHS /usr/local /usr /sw /opt/local /opt )

SET(DLOPEN_FOUND "NO")

IF(DLOPEN_INCLUDE_DIR AND DLOPEN_LIBRARY)
   SET(DLOPEN_FOUND TRUE)
ELSE(DLOPEN_INCLUDE_DIR AND DLOPEN_LIBRARY)
   UNSET(DLOPEN_LIBRARY)
   UNSET(DLOPEN_INCLUDE_DIR)
ENDIF(DLOPEN_INCLUDE_DIR AND DLOPEN_LIBRARY)

IF(DLOPEN_FOUND)
  MESSAGE(STATUS "  OK : dlopen found: ${DLOPEN_LIBRARY}")
ELSE(DLOPEN_FOUND)
  MESSAGE(STATUS "! KO : dlopen NOT found")
ENDIF(DLOPEN_FOUND)

