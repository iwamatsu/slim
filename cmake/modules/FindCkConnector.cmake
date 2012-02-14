# - Try to find the ConsoleKit connector library (libck-connector)
# Once done this will define
#
#  CKCONNECTOR_FOUND - system has the CK Connector
#  CKCONNECTOR_INCLUDE_DIR - the CK Connector include directory
#  CKCONNECTOR_LIBRARIES - the libraries needed to use CK Connector

# Copyright (c) 2008, Kevin Kofler, <kevin.kofler@chello.at>
# modeled after FindLibArt.cmake:
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (CKCONNECTOR_INCLUDE_DIR AND CKCONNECTOR_LIBRARIES)

  # in cache already
  SET(CKCONNECTOR_FOUND TRUE)

else (CKCONNECTOR_INCLUDE_DIR AND CKCONNECTOR_LIBRARIES)

  IF (NOT WIN32)
    FIND_PACKAGE(PkgConfig)
    IF (PKG_CONFIG_FOUND)
      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      pkg_check_modules(_CKCONNECTOR_PC QUIET ck-connector)
    ENDIF (PKG_CONFIG_FOUND)
  ENDIF (NOT WIN32)

  FIND_PATH(CKCONNECTOR_INCLUDE_DIR ck-connector.h
     ${_CKCONNECTOR_PC_INCLUDE_DIRS}
  )

  FIND_LIBRARY(CKCONNECTOR_LIBRARIES NAMES ck-connector
     PATHS
     ${_CKCONNECTOR_PC_LIBDIR}
  )


  if (CKCONNECTOR_INCLUDE_DIR AND CKCONNECTOR_LIBRARIES)
     set(CKCONNECTOR_FOUND TRUE)
  endif (CKCONNECTOR_INCLUDE_DIR AND CKCONNECTOR_LIBRARIES)


  if (CKCONNECTOR_FOUND)
     if (NOT CkConnector_FIND_QUIETLY)
        message(STATUS "Found ck-connector: ${CKCONNECTOR_LIBRARIES}")
     endif (NOT CkConnector_FIND_QUIETLY)
  else (CKCONNECTOR_FOUND)
     if (CkConnector_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find ck-connector")
     endif (CkConnector_FIND_REQUIRED)
  endif (CKCONNECTOR_FOUND)

  MARK_AS_ADVANCED(CKCONNECTOR_INCLUDE_DIR CKCONNECTOR_LIBRARIES)

endif (CKCONNECTOR_INCLUDE_DIR AND CKCONNECTOR_LIBRARIES)
