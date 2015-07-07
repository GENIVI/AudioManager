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


set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})


if(USE_BUILD_LIBS) 
     execute_process(COMMAND mkdir -p "${EXECUTABLE_OUTPUT_PATH}")
endif(USE_BUILD_LIBS)

#make aunt google stop complaining
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")

add_subdirectory (AmControlInterfaceTest)
add_subdirectory (AmDatabaseHandlerTest)
add_subdirectory (AmMapHandlerTest)
add_subdirectory (AmRouterTest)
add_subdirectory (AmRouterMapTest)
add_subdirectory (AmRoutingInterfaceTest)
add_subdirectory (AmSocketHandlerTest)

IF(WITH_NSM)
    IF(WITH_DBUS_WRAPPER)
        add_subdirectory (AmNodeStateCommunicatorDBusTest)
    ELSEIF(WITH_CAPI_WRAPPER)
        add_subdirectory (AmNodeStateCommunicatorCAPITest)
    ENDIF()
ENDIF(WITH_NSM)

IF(WITH_TELNET)
    add_subdirectory (AmTelnetServerTest)
ENDIF(WITH_TELNET)

