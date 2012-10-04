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

PROJECT(AmTelnetServerTest)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")

set(STD_INCLUDE_DIRS "/usr/include")
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../../../dbusInterfaces)
set(AUDIO_INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/../../../include)
set(DBUS_INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/../../../../dbusinclude)
set(INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/../../include)


FIND_PACKAGE(PkgConfig)

IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
ENDIF(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_INCLUDE_FOLDER} 
    ${CMAKE_SOURCE_DIR} 
    ${STD_INCLUDE_DIRS}
    ${DLT_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIR}
    ${INCLUDE_FOLDER}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
)
 
file(GLOB TELNET_SRCS_CXX 
    "../../src/CAmCommandReceiver.cpp"
    "../../src/CAmCommandSender.cpp"
    "../../src/CAmControlReceiver.cpp"
    "../../src/CAmControlSender.cpp"
    "../../src/CAmDatabaseHandler.cpp"
    "../../src/CAmDatabaseObserver.cpp"
    "../../src/CAmRoutingReceiver.cpp"
    "../../src/CAmRoutingSender.cpp"
    "../../src/CAmSocketHandler.cpp"
    "../../src/CAmTelnetServer.cpp"
    "../../src/CAmRouter.cpp"
    "../../src/CAmDltWrapper.cpp"
    "../../src/CAmTelnetMenuHelper.cpp"
    "*.cpp"    
)

ADD_EXECUTABLE(AmTelnetServerTest ${TELNET_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmTelnetServerTest 
    ${SQLITE_LIBRARIES}
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}    
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_THREAD_LIBS_INIT}
    gtest
    gmock
)

ADD_DEPENDENCIES(AmTelnetServerTest gtest gmock)

INSTALL(TARGETS AmTelnetServerTest 
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

SET(ADD_DEPEND "sqlite3(>=3.6.22)" "dlt" "libdbus-1-3(>=1.2.16)"  "libpthread-stubs0")
set_property(GLOBAL APPEND PROPERTY tests_prop "${ADD_DEPEND}")


