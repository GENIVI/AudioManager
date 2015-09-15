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

project(AmControlInterfaceTest)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")

set (AM_LINK_LIBS 
    ${CMAKE_DL_LIBS}
    AudioManagerUtilities
    rt)

if(WITH_DATABASE_STORAGE)
	pkg_check_modules(SQLITE REQUIRED sqlite3)
	set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${SQLITE_LIBRARIES})
endif(WITH_DATABASE_STORAGE)

if(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${DLT_LIBRARIES})
endif(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${STD_INCLUDE_DIRS}
    ${INCLUDE_FOLDER}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR})
    
if(WITH_DLT)    
    INCLUDE_DIRECTORIES(
        ${INCLUDE_DIRECTORIES} 
        ${DLT_INCLUDE_DIRS})
endif(WITH_DLT)

   
file(GLOB CONTROL_INTERFACE_SRCS_CXX 
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmDatabaseHandlerMap.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmDatabaseObserver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRoutingSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRoutingReceiver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmCommandSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmCommandReceiver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmControlReceiver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmControlSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRouter.cpp"
    "../CAmCommonFunctions.cpp" 
    "*.cpp"    
)

add_executable(AmControlInterfaceTest ${CONTROL_INTERFACE_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmControlInterfaceTest 
    ${AM_LINK_LIBS}
    gtest
    gmock)

ADD_DEPENDENCIES(AmControlInterfaceTest gtest gmock AudioManagerUtilities)

INSTALL(TARGETS AmControlInterfaceTest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests)




