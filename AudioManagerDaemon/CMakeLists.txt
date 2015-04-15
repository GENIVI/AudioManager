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

cmake_minimum_required(VERSION 2.8.8)

project (AudioManagerDeamon)

set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/AudioManager)

set(AM_MAP_CAPACITY 10
    CACHE INTEGER "Number of preallocations for datastorage")

set(AM_MAX_CONNECTIONS 0x1000
    CACHE INTEGER "Number of max connections before rollover")

set(AM_MAX_CONNECTIONS SHRT_MAX
    CACHE INTEGER "Number of max Mainconnections before rollover")   

set(INCLUDE_FOLDER "include")

if (WITH_DATABASE_STORAGE)
    pkg_check_modules(SQLITE REQUIRED sqlite3)
endif (WITH_DATABASE_STORAGE)

if (WITH_SYSTEMD_WATCHDOG)
	pkg_check_modules(SYSTEMD libsystemd-daemon>=44)
	if (SYSTEMD_FOUND)
		message(STATUS "Found libsystemd-daemon, version: ${SYSTEMD_VERSION}")
	else (SYSTEMD_FOUND)
		message(STATUS "Did not find libsystemd-daemon, using own lib")
	endif(SYSTEMD_FOUND)
endif (WITH_SYSTEMD_WATCHDOG)

if(WITH_DBUS_WRAPPER)
    find_package(DBUS REQUIRED)
endif(WITH_DBUS_WRAPPER)

if(WITH_CAPI_WRAPPER)
    find_package(CommonAPI REQUIRED)
endif(WITH_CAPI_WRAPPER)

set (AM_LINK_LIBS 
    ${CMAKE_DL_LIBS}
    rt)

set(AUDIOMAN_SRCS_CXX
    src/main.cpp
    src/CAmCommandReceiver.cpp
    src/CAmCommandSender.cpp
    src/CAmControlReceiver.cpp
    src/CAmControlSender.cpp
    src/CAmDatabaseObserver.cpp
    src/CAmRoutingReceiver.cpp
    src/CAmRoutingSender.cpp
    src/CAmRouter.cpp
    src/CAmLog.cpp
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmSocketHandler.cpp"
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmDltWrapper.cpp"
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmCommandLineSingleton.cpp")

if(WITH_TELNET)
	set (AUDIOMAN_SRCS_CXX
		${AUDIOMAN_SRCS_CXX}
		src/CAmTelnetServer.cpp
		src/CAmTelnetMenuHelper.cpp)		
endif(WITH_TELNET)		

if(WITH_SYSTEMD_WATCHDOG)
	set (AUDIOMAN_SRCS_CXX
		${AUDIOMAN_SRCS_CXX}
		src/CAmWatchdog.cpp)
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${SYSTEMD_LIBRARIES) 
endif(WITH_SYSTEMD_WATCHDOG)

if (NOT SYSTEMD_FOUND)
	set (AUDIOMAN_SRCS_CXX
		${AUDIOMAN_SRCS_CXX}
		src/sd-daemon.c)
endif (NOT SYSTEMD_FOUND)

if(WITH_DBUS_WRAPPER)
    set (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        "${AUDIOMANAGER_UTILITIES_SRC}/CAmDbusWrapper.cpp")
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${DBUS_LIBRARY})
endif(WITH_DBUS_WRAPPER)

if(WITH_CAPI_WRAPPER)
    set (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        "${AUDIOMANAGER_UTILITIES_SRC}/CAmCommonAPIWrapper.cpp")
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${DBUS_LIBRARY}     
        ${COMMON_API_LIBRARIES}
        ${COMMON_API_DBUS_LIBRARIES})
endif(WITH_CAPI_WRAPPER)

if(WITH_NSM)
    if(WITH_DBUS_WRAPPER)
        set (AUDIOMAN_SRCS_CXX
            ${AUDIOMAN_SRCS_CXX}
            src/CAmNodeStateCommunicatorDBus.cpp)
    else(WITH_DBUS_WRAPPER)
            set (AUDIOMAN_SRCS_CXX
            ${AUDIOMAN_SRCS_CXX}
            src/CAmNodeStateCommunicatorCAPI.cpp)
    endif(WITH_DBUS_WRAPPER)
endif(WITH_NSM)

if(WITH_DATABASE_STORAGE)
    set (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmDatabaseHandlerSQLite.cpp)
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${SQLITE_LIBRARIES})
else(WITH_DATABASE_STORAGE)
    set (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmDatabaseHandlerMap.cpp)
endif(WITH_DATABASE_STORAGE)

INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}	
	${AUDIO_INCLUDE_FOLDER}
	${AUDIOMANAGER_INCLUDE_FOLDER}
	${AUDIOMANAGER_UTILITIES_INCLUDE}
	${INCLUDE_FOLDER}
	${NSM_INCLUDE_DIR}
	${TCLAP_INCLUDE_DIR}
)

if(WITH_CAPI_WRAPPER)
    
    if(WITH_NSM)
        include (CommonAPIGenerator)
        COMMON_API_GENERATE_SOUCRES(TARGET COMMON_API 
                            FIDLS fidls/NodeStateManager.fidl # a list with fidls
                            DESTINATION src-gen  # a relative path to the build directory or an absolute path
                            ALT_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src-gen # an alternative relative/absolute path with common-api sources, usually in the source tree
                            DBUS)
        set(AUDIOMAN_SRCS_CXX 
            ${AUDIOMAN_SRCS_CXX} 
            ${COMMON_API_GEN_SOURCES})
    endif(WITH_NSM)

    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
            ${COMMON_API_INCLUDE_DIRS}
	        ${COMMON_API_DBUS_INCLUDE_DIRS}
	        ${COMMON_API_GEN_INCLUDE_DIR})
	        
endif(WITH_CAPI_WRAPPER)

if(WITH_DLT)
    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
        ${DLT_INCLUDE_DIRS})
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${DLT_LIBRARIES})
endif(WITH_DLT)

if(WITH_DBUS_WRAPPER)
    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_INCLUDE_DIR})
endif(WITH_DBUS_WRAPPER)

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX})

TARGET_LINK_LIBRARIES(AudioManager ${AM_LINK_LIBS})

INSTALL(TARGETS AudioManager 
        RUNTIME DESTINATION bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT bin)

INSTALL(DIRECTORY "${AUDIO_INCLUDE_FOLDER}/" 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIB_INSTALL_SUFFIX}
        COMPONENT dev)

INSTALL(DIRECTORY "${AUDIOMANAGER_UTILITIES_INCLUDE}/" 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIB_INSTALL_SUFFIX}
        COMPONENT dev)        

if(WITH_TESTS)
	add_subdirectory (test)
endif(WITH_TESTS)



