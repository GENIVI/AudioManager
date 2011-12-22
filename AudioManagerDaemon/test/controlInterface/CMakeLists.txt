#
# Copyright (C) 2011, BMW AG
#
# GeniviAudioMananger ControlInterfaceTest
#
# \file CMakeLists.txt
#
# \date 20-Oct-2011 3:42:04 PM
# \author Christian Mueller (christian.ei.mueller@bmw.de)
#
# \section License
# GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
# Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
#
# This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
# You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
# Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
# Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
# As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
# Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
#

cmake_minimum_required(VERSION 2.6)

PROJECT(controlInterfacetest)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")

FIND_PACKAGE(PkgConfig)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(DLT REQUIRED automotive-dlt)

INCLUDE_DIRECTORIES(   
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDES_FOLDER}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_INCLUDES_FOLDER} 
    ${CMAKE_SOURCE_DIR} 
    ${STD_INCLUDE_DIRS}
    ${DLT_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}
    ${INCLUDES_FOLDER}
)
   
file(GLOB CONTROL_INTERFACE_SRCS_CXX 
    "../../src/DatabaseHandler.cpp"
    "../../src/DatabaseObserver.cpp"
    "../../src/RoutingSender.cpp"
    "../../src/RoutingReceiver.cpp"
    "../../src/CommandSender.cpp"
    "../../src/ControlReceiver.cpp"
    "../../src/ControlSender.cpp"
    "../CommonFunctions.cpp" 
    "*.cpp"    
)

ADD_EXECUTABLE(controlInterfaceTest ${CONTROL_INTERFACE_SRCS_CXX})

TARGET_LINK_LIBRARIES(controlInterfaceTest 
    ${SQLITE_LIBRARIES}
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GTEST_LIBRARIES}
    gmock
)



