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
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 2.8.8)

project (AmRouterMapTest)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")

set (AM_LINK_LIBS 
    ${CMAKE_DL_LIBS}
    rt)
    
if(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
    set (AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${DLT_LIBRARIES})
endif(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${CMAKE_SOURCE_DIR} 
    ${INCLUDE_FOLDER}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
)

file(GLOB ROUTINGMAP_SRCS_CXX 
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmDatabaseHandlerMap.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmDatabaseObserver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmCommandSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRoutingSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmControlReceiver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmControlSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRouter.cpp"
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmDltWrapper.cpp"
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmSocketHandler.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmCommandReceiver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRoutingReceiver.cpp"
     "../CAmCommonFunctions.cpp" 
    "*.cpp"
    )
    
ADD_EXECUTABLE( AmRouterMapTest ${ROUTINGMAP_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmRouterMapTest 
    ${AM_LINK_LIBS}
    gtest
    gmock)

ADD_DEPENDENCIES(AmRouterMapTest gtest gmock)

INSTALL(TARGETS AmRouterMapTest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

