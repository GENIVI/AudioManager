# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. if a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 3.0)

project (AudioManagerCore LANGUAGES CXX VERSION ${DAEMONVERSION})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET(AUDIOMANAGER_CORE_INCLUDE
	${AUDIOMANAGER_CORE}/include
	${AUDIOMANAGER_UTILITIES_INCLUDE}
	CACHE INTERNAL "AUDIOMANAGER_CORE_INCLUDE directories" FORCE)

SET(AUDIO_MANAGER_CORE_LIBS
	${AUDIO_MANAGER_UTILITIES_LIBS}
	AudioManagerUtilities
	CACHE INTERNAL "AUDIO_MANAGER_CORE_LIBS libs" FORCE)

SET(AUDIO_MANAGER_CORE_LIB_DIRECTORIES
	${AUDIOMANAGER_UTILITIES_LIB_DIRECTORIES}
	CACHE INTERNAL "AUDIO_MANAGER_CORE_LIB_DIRECTORIES directories" FORCE)

IF (WITH_SHARED_CORE)
    SET(LIBRARY_TYPE SHARED)
ELSE (WITH_SHARED_CORE)
    SET(LIBRARY_TYPE STATIC)
ENDIF (WITH_SHARED_CORE)

set(AUDIOMAN_CORE_SRCS_CXX
    src/CAmCommandReceiver.cpp
    src/CAmCommandSender.cpp
    src/CAmControlReceiver.cpp
    src/CAmControlSender.cpp
    src/CAmRoutingReceiver.cpp
    src/CAmRoutingSender.cpp
    src/CAmRouter.cpp
    src/CAmLog.cpp
    src/CAmDatabaseHandlerMap.cpp
)

if(WITH_TELNET)
	set (AUDIOMAN_CORE_SRCS_CXX
		${AUDIOMAN_CORE_SRCS_CXX}
		src/CAmTelnetServer.cpp
		src/CAmTelnetMenuHelper.cpp)
endif(WITH_TELNET)

INCLUDE_DIRECTORIES(${AUDIOMANAGER_CORE_INCLUDE})

LINK_DIRECTORIES(${AUDIO_MANAGER_CORE_LIB_DIRECTORIES})

ADD_LIBRARY(AudioManagerCore ${LIBRARY_TYPE} ${AUDIOMAN_CORE_SRCS_CXX})

TARGET_LINK_LIBRARIES(AudioManagerCore ${AUDIO_MANAGER_CORE_LIBS} )

ADD_DEPENDENCIES(AudioManagerCore AudioManagerUtilities)

set_target_properties(AudioManagerCore PROPERTIES VERSION ${AudioManagerCore_VERSION} SOVERSION ${AudioManagerCore_VERSION_MAJOR})


IF (WITH_SHARED_CORE)
	INSTALL(TARGETS AudioManagerCore
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
		COMPONENT bin)
ELSE (WITH_SHARED_CORE)
	INSTALL(TARGETS AudioManagerCore
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	        COMPONENT dev)
ENDIF (WITH_SHARED_CORE)

configure_file( ${CMAKE_SOURCE_DIR}/cmake/audiomanagercore.pc.in ${CMAKE_BINARY_DIR}/audiomanagercore.pc @ONLY )
install(FILES ${CMAKE_BINARY_DIR}/audiomanagercore.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)

configure_package_config_file (
	${CMAKE_SOURCE_DIR}/cmake/AudioManagerCoreConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/AudioManagerCoreConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUFFIX}/cmake
	PATH_VARS AUDIO_INCLUDE_FOLDER )

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/AudioManagerCoreConfigVersion.cmake
	VERSION ${DAEMONVERSION}
	COMPATIBILITY SameMajorVersion )

install(
FILES ${CMAKE_CURRENT_BINARY_DIR}/AudioManagerCoreConfig.cmake
	  ${CMAKE_CURRENT_BINARY_DIR}/AudioManagerCoreConfigVersion.cmake
      DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/AudioManagerCore)

INSTALL(DIRECTORY "${AUDIOMANAGER_CORE}/include/"
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${LIB_INSTALL_SUFFIX}/AudioManagerCore
        COMPONENT dev)

if(WITH_TESTS)
	add_subdirectory (test)
endif(WITH_TESTS)



