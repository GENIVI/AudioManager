# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 2.6)

PROJECT(AudioManagerDeamon)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/AudioManager)
set(INCLUDE_FOLDER "include")

pkg_check_modules(SQLITE REQUIRED sqlite3)

IF (WITH_SYSTEMD_WATCHDOG)
	pkg_check_modules(SYSTEMD libsystemd-daemon>=44)
	IF (SYSTEMD_FOUND)
		message(STATUS "Found libsystemd-daemon, version: ${SYSTEMD_VERSION}")
	ELSE (SYSTEMD_FOUND)
		message(STATUS "Did not find libsystemd-daemon, using own lib")
	ENDIF(SYSTEMD_FOUND)
ENDIF (WITH_SYSTEMD_WATCHDOG)

IF(WITH_DBUS_WRAPPER)
    FIND_PACKAGE(DBUS REQUIRED)
ENDIF(WITH_DBUS_WRAPPER)

SET(AUDIOMAN_SRCS_CXX
    src/main.cpp
    src/CAmCommandReceiver.cpp
    src/CAmCommandSender.cpp
    src/CAmControlReceiver.cpp
    src/CAmControlSender.cpp
    src/CAmDatabaseHandler.cpp
    src/CAmDatabaseObserver.cpp
    src/CAmRoutingReceiver.cpp
    src/CAmRoutingSender.cpp
    src/CAmSocketHandler.cpp
    src/CAmTelnetServer.cpp
    src/CAmRouter.cpp
    src/CAmDltWrapper.cpp
    src/CAmTelnetMenuHelper.cpp
    src/CAmWatchdog.cpp
)

IF (NOT SYSTEMD_FOUND)
	SET (AUDIOMAN_SRCS_CXX
		${AUDIOMAN_SRCS_CXX}
		src/sd-daemon.c
	)
ENDIF (NOT SYSTEMD_FOUND)

IF(WITH_DBUS_WRAPPER)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmDbusWrapper.cpp
    )
ENDIF(WITH_DBUS_WRAPPER)

IF(WITH_SOCKETHANDLER_LOOP)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmSocketHandler.cpp
     )
ENDIF(WITH_SOCKETHANDLER_LOOP)

INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}	
	${AUDIO_INCLUDE_FOLDER}
	${PROJECT_INCLUDE_FOLDER}
	${INCLUDE_FOLDER}
)

IF(WITH_DLT)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${DLT_INCLUDE_DIRS}
)
ENDIF(WITH_DLT)

IF(WITH_DBUS_WRAPPER)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_INCLUDE_DIR}
)
ENDIF(WITH_DBUS_WRAPPER)

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX})

IF(WITH_DBUS_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
    	${SQLITE_LIBRARIES}
    	${CMAKE_DL_LIBS}
    	${DBUS_LIBRARY}
    	rt
    )
ELSE(WITH_DBUS_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
        ${SQLITE_LIBRARIES}
        ${CMAKE_DL_LIBS}
        rt
    )
ENDIF(WITH_DBUS_WRAPPER)

IF(WITH_DLT)
    TARGET_LINK_LIBRARIES(AudioManager
        ${DLT_LIBRARIES} 
    )
ENDIF(WITH_DLT) 

IF (SYSTEMD_FOUND)
	TARGET_LINK_LIBRARIES(AudioManager
        ${SYSTEMD_LIBRARIES} 
	)
ENDIF (SYSTEMD_FOUND)

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

#IF(WITH_DOCUMENTATION)
#	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
#	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
#  	add_custom_target (Docs ALL 
#  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
#  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
#	)
#ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS AudioManager 
        RUNTIME 
        DESTINATION bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT bin
)

INSTALL(DIRECTORY "${AUDIO_INCLUDE_FOLDER}/" 
        DESTINATION include
        COMPONENT dev
)

# General packet dependency
SET(ADD_DEPEND_BIN_PROP "sqlite3(>=3.6.22)")

# Optional packet dependency
IF(WITH_DLT)
    SET(ADD_DEPEND_BIN_PROP ${ADD_DEPEND_BIN_PROP} "dlt")
ENDIF(WITH_DLT)

IF(WITH_DBUS_WRAPPER)
    SET(ADD_DEPEND_BIN_PROP ${ADD_DEPEND_BIN_PROP} "libdbus-1-3(>=1.2.16)")
ENDIF()

# Additional Dev packet dependency
SET(ADD_DEPEND_DEV_PROP "audiomanager-bin" ${ADD_DEPEND_BIN_PROP})

MESSAGE(STATUS "bin packet dependency: ${ADD_DEPEND_BIN_PROP}")
MESSAGE(STATUS "dev dependency: ${ADD_DEPEND_DEV_PROP}")

set_property(GLOBAL APPEND PROPERTY bin_prop "${ADD_DEPEND_BIN_PROP}")
set_property(GLOBAL APPEND PROPERTY dev_prop "${ADD_DEPEND_DEV_PROP}")



