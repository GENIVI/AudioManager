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

IF(WITH_CAPI_WRAPPER)
    FIND_PACKAGE(CommonAPI REQUIRED)
ENDIF(WITH_CAPI_WRAPPER)

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
    src/CAmLog.cpp
)

# Preallocation variable for the map.
IF(NOT WITH_DATABASE_STORAGE )

    IF(NOT DEFINED AM_MAP_CAPACITY)
        set (AM_MAP_CAPACITY 10)
    ENDIF(NOT DEFINED AM_MAP_CAPACITY)
    add_definitions( -DAM_MAP_CAPACITY=${AM_MAP_CAPACITY} )

    IF(NOT DEFINED AM_MAX_CONNECTIONS)
        set (AM_MAX_CONNECTIONS 0x1000)
    ENDIF(NOT DEFINED AM_MAX_CONNECTIONS)
    add_definitions( -DAM_MAX_CONNECTIONS=${AM_MAX_CONNECTIONS} )
    
    IF(NOT DEFINED AM_MAX_MAIN_CONNECTIONS)
        set (AM_MAX_MAIN_CONNECTIONS SHRT_MAX)
    ENDIF(NOT DEFINED AM_MAX_MAIN_CONNECTIONS)
    add_definitions( -DAM_MAX_MAIN_CONNECTIONS=${AM_MAX_MAIN_CONNECTIONS} )

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

IF(WITH_CAPI_WRAPPER)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmCommonAPIWrapper.cpp
    )
ENDIF(WITH_CAPI_WRAPPER)

IF(WITH_NSM)
    IF(WITH_DBUS_WRAPPER)
        SET (AUDIOMAN_SRCS_CXX
            ${AUDIOMAN_SRCS_CXX}
            src/CAmNodeStateCommunicatorDBus.cpp
        )
    ELSE(WITH_DBUS_WRAPPER)
            SET (AUDIOMAN_SRCS_CXX
            ${AUDIOMAN_SRCS_CXX}
            src/CAmNodeStateCommunicatorCAPI.cpp
        )
    ENDIF(WITH_DBUS_WRAPPER)
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

IF(WITH_CAPI_WRAPPER)

# Find the common-api sources. 
# The sources can be placed in the build directory (DESTINATION) or in the source tree (ALT_DESTINATION). 
# The ALT_DESTINATION is used as fall back if the DESTINATION directory dosen't contain the sources or you just don't want to use the generator.
INCLUDE(CommonAPIGenerator)
COMMON_API_GENERATE_SOUCRES(TARGET COMMON_API 
                            FIDLS fidls/NodeStateManager.fdepl # a list with fidls
                            DESTINATION src-gen  # a relative path to the build directory or an absolute path
                            ALT_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src-gen # an alternative relative/absolute path with common-api sources, usually in the source tree
                            DBUS)

INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${COMMON_API_INCLUDE_DIRS}
	${COMMON_API_DBUS_INCLUDE_DIRS}
	${COMMON_API_GEN_INCLUDE_DIR}
)
ENDIF(WITH_CAPI_WRAPPER)

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

file(GLOB ${AUDIOMAN_SRCS_CXX} ${COMMON_API_GEN_SOURCES})

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX} ${COMMON_API_GEN_SOURCES})

TARGET_LINK_LIBRARIES(AudioManager 
	${SQLITE_LIBRARIES}
	${CMAKE_DL_LIBS}
 )
 
 IF(WITH_DBUS_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
    	${DBUS_LIBRARY}
    	rt
    )
ELSEIF(WITH_CAPI_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
        ${COMMON_API_LIBRARIES}
        ${COMMON_API_DBUS_LIBRARIES}
        rt
    )
ENDIF()

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

IF(WITH_CAPI_WRAPPER)
    IF(WITH_NSM)
        INSERT_DBUS_CONF_IF_NEEDED("@NSM_DBUS_CONF@" 
                                  "${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/NodeStateManager_dbus.conf" 
                                  NSM_DBUS_CONF)
    ENDIF(WITH_NSM)
# Generates the dbus configuration file. Every component is responsible to append it dbus configuration part.
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf)
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in ${CMAKE_BINARY_DIR}/AudioManager_dbus.conf)

ENDIF(WITH_CAPI_WRAPPER)


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
    	    COMMAND cp "${CMAKE_SOURCE_DIR}/cmake/LifecycleConsumer.xml" ${NSM_INTROSPECTION_FILE}
    	    COMMAND cp "${CMAKE_BINARY_DIR}/AudioManager_dbus.conf" ${EXECUTABLE_OUTPUT_PATH})
    ELSE(USE_BUILD_LIBS) 
        SET(NSM_INTROSPECTION_FILE ${CMAKE_INSTALL_PREFIX}/share/audiomanager/LifecycleConsumer.xml)
        INSTALL(
            FILES ${CMAKE_SOURCE_DIR}/cmake/LifecycleConsumer.xml #${CMAKE_BINARY_DIR}/AudioManager_dbus.conf
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
ENDIF(WITH_DBUS_WRAPPER)

# Additional Dev packet dependency
SET(ADD_DEPEND_DEV_PROP "audiomanager-bin" ${ADD_DEPEND_BIN_PROP})

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -fno-common -fno-strict-aliasing -DNDEBUG")

MESSAGE(STATUS "bin packet dependency: ${ADD_DEPEND_BIN_PROP}")
MESSAGE(STATUS "dev dependency: ${ADD_DEPEND_DEV_PROP}")

set_property(GLOBAL APPEND PROPERTY bin_prop "${ADD_DEPEND_BIN_PROP}")
set_property(GLOBAL APPEND PROPERTY dev_prop "${ADD_DEPEND_DEV_PROP}")



