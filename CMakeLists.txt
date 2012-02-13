cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

		set(PROJECT_NAME slim)
project(${PROJECT_NAME})

#Pretty colors
set(CMAKE_COLOR_MAKEFILE ON)
#Dont force verbose
set(CMAKE_VERBOSE_MAKEFILE ON)
#Include current dir
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckTypeSize)

# Version
set(SLIM_VERSION_MAJOR "1")
set(SLIM_VERSION_MINOR "3")
set(SLIM_VERSION_PATCH "2")
set(SLIM_VERSION "${SLIM_VERSION_MAJOR}.${SLIM_VERSION_MINOR}.${SLIM_VERSION_PATCH}")

set(CMAKE_INSTALL_PREFIX "/usr/share" CACHE PATH "Installation Directory")
set(PKGDATADIR "${CMAKE_INSTALL_PREFIX}/share/slim")
set(SYSCONFDIR "/etc")
set(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man")

set(SLIM_DEFINITIONS)
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" OR
		${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" OR
		${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD"
		)
	set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DNEEDS_BASENAME")
else()
	set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DHAVE_SHADOW")
endif()

set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DPACKAGE=\"slim\"")
set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DVERSION=\"${SLIM_VERSION}\"")
set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DPKGDATADIR=\"${PKGDATADIR}\"")
set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DSYSCONFDIR=\"${SYSCONFDIR}\"")

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security" )
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -Wall -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Werror=format-security" )

# source 
set(slim_srcs
	main.cpp
	app.cpp
	cfg.cpp
	image.cpp
	numlock.cpp
	panel.cpp
	switchuser.cpp
	util.cpp
	PAM.cpp
	png.c
	jpeg.c
)

add_executable(${PROJECT_NAME} ${slim_srcs})

#Set the custom CMake module directory where our include/lib finders are
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(X11 REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLUT REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

set(FONTCONFIG_DIR ${CMAKE_MODULE_PATH})

# Fontconfig
find_package(FONTCONFIG REQUIRED)
if(FONTCONFIG_FOUND)
	message("\tFontConfig Found")
	target_link_libraries(${PROJECT_NAME} ${FONTCONFIG_LIBRARY})
	include_directories(${FONTCONFIG_INCLUDE_DIR})
endif(FONTCONFIG_FOUND)

# PAM
if(USE_PAM)
	set(SLIM_DEFINITIONS ${SLIM_DEFINITIONS} "-DUSE_PAM")
	find_package(PAM)
	if(PAM_FOUND)
		message("\tPAM Enabled")
		message("\tPAM Found")
		target_link_libraries(${PROJECT_NAME} ${PAM_LIBRARY})
		include_directories(${PAM_INCLUDE_DIR})
	endif(PAM_FOUND)
else(USE_PAM)
	message("\tPAM disabled")
endif(USE_PAM)

# system librarys
find_library(M_LIB m)
find_library(RT_LIB rt)
find_library(CRYPTO_LIB crypt)

add_definitions(${SLIM_DEFINITIONS})

#Set up include dirs with all found packages
include_directories(
	${X11_INCLUDE_DIR}
	${X11_Xft_INCLUDE_PATH}
	${X11_Xrender_INCLUDE_PATH}
	${FREETYPE_INCLUDE_DIR_freetype2}
	${GLUT_Xmu_INCLUDE_PATH}
	${ZLIB_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
)

#Set up library with all found packages
target_link_libraries(${PROJECT_NAME}
	${M_LIB}
	${RT_LIB}
	${CRYPTO_LIB}
	${X11_X11_LIB}
	${X11_Xft_LIB}
	${X11_Xrender_LIB}
	${FREETYPE_LIBRARY}
	${GLUT_Xmu_LIBRARY}
	${JPEG_LIBRARIES}
	${PNG_LIBRARIES}
	)

# install
# slim
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin )
# man file
install(FILES slim.1 DESTINATION ${MANDIR}/man1/)
# configure
install(FILES slim.conf DESTINATION ${SYSCONFDIR})
# themes directory
subdirs(themes)
