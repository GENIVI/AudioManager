# Copyright (C) 2012, GENIVI Alliance, Inc.
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
set(AUDIO_include_FOLDER ${CMAKE_SOURCE_DIR}/../../../include)
set(DBUS_include_FOLDER ${CMAKE_SOURCE_DIR}/../../../../dbusinclude)
set(include_FOLDER ${CMAKE_SOURCE_DIR}/../../include)


FIND_PACKAGE(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)

INCLUDE_DIRECTORIES(   
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_include_FOLDER}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_include_FOLDER} 
    ${CMAKE_SOURCE_DIR} 
    ${STD_INCLUDE_DIRS}
    ${DLT_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}
    ${include_FOLDER}
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
    ${GTEST_LIBRARIES}
    gmock
)

INSTALL(TARGETS AmTelnetServerTest 
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

SET(ADD_DEPEND "sqlite3(>=3.6.22)" "dlt" "libdbus-1-3(>=1.2.16)" "libgtest-dev(>=1.6.0-1)" "google-mock" "libpthread-stubs0")
set_property(GLOBAL APPEND PROPERTY tests_prop "${ADD_DEPEND}")


