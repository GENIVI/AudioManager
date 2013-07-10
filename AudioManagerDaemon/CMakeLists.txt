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

IF (WITH_DATABASE_STORAGE)
    pkg_check_modules(SQLITE REQUIRED sqlite3)
ENDIF (WITH_DATABASE_STORAGE)

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

FIND_PACKAGE(NSM REQUIRED)

SET(AUDIOMAN_SRCS_CXX
    src/main.cpp
    src/CAmCommandReceiver.cpp
    src/CAmCommandSender.cpp
    src/CAmControlReceiver.cpp
    src/CAmControlSender.cpp
    src/CAmDatabaseObserver.cpp
    src/CAmRoutingReceiver.cpp
    src/CAmRoutingSender.cpp
    src/CAmRouter.cpp
    src/CAmSocketHandler.cpp
    src/CAmDltWrapper.cpp
)

# Preallocation variable for the map.
IF(NOT WITH_DATABASE_STORAGE )
    set (AM_MAP_CAPACITY 10)
    add_definitions( -DAM_MAP_CAPACITY=${AM_MAP_CAPACITY} )
ENDIF(NOT WITH_DATABASE_STORAGE)	

IF(WITH_TELNET)
	SET (AUDIOMAN_SRCS_CXX
		${AUDIOMAN_SRCS_CXX}
		src/CAmTelnetServer.cpp
		src/CAmTelnetMenuHelper.cpp
     )		
ENDIF(WITH_TELNET)		

IF(WITH_SYSTEMD_WATCHDOG)
	SET (AUDIOMAN_SRCS_CXX
		${AUDIOMAN_SRCS_CXX}
		src/CAmWatchdog.cpp
     )		
ENDIF(WITH_SYSTEMD_WATCHDOG)

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

IF(WITH_NSM)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmNodeStateCommunicator.cpp
    )
ENDIF(WITH_NSM)

IF(WITH_DATABASE_STORAGE)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmDatabaseHandlerSQLite.cpp
    )
ELSE(WITH_DATABASE_STORAGE)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmDatabaseHandlerMap.cpp
    )
ENDIF(WITH_DATABASE_STORAGE)

INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}	
	${AUDIO_INCLUDE_FOLDER}
	${PROJECT_INCLUDE_FOLDER}
	${INCLUDE_FOLDER}
	${NSM_INCLUDE_DIR}
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

IF(WITH_NSM)
    IF(USE_BUILD_LIBS) 
        SET(NSM_INTROSPECTION_FILE ${EXECUTABLE_OUTPUT_PATH}/LifeCycleConsumer.xml)
            execute_process(
    	    COMMAND mkdir -p "${CMAKE_SOURCE_DIR}/bin"
    	    COMMAND cp "${CMAKE_SOURCE_DIR}/cmake/LifecycleConsumer.xml" ${NSM_INTROSPECTION_FILE})
    ELSE(USE_BUILD_LIBS) 
        SET(NSM_INTROSPECTION_FILE ${CMAKE_INSTALL_PREFIX}/share/audiomanager/LifeCycleConsumer.xml)
        INSTALL(
            FILES ${CMAKE_SOURCE_DIR}/cmake/LifecycleConsumer.xml
            DESTINATION "${NSM_INTROSPECTION_FILE}"
            COMPONENT bin)
    ENDIF(USE_BUILD_LIBS)
ENDIF(WITH_NSM)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/cmake/config.cmake ${CMAKE_SOURCE_DIR}/include/config.h )

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

# General packet dependency
IF(WITH_DATABASE_STORAGE)
    SET(ADD_DEPEND_BIN_PROP "sqlite3(>=3.6.22)")
ENDIF(WITH_DATABASE_STORAGE)

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



