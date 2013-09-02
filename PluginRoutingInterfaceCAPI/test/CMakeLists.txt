# Copyright (c) 2012 GENIVI Alliance
# Copyright (c) 2012 BMW
#
# author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
#
# copyright
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 2.6)

PROJECT(CAmRoutingInterfaceCAPITests)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -g -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")

string (REPLACE "-g -Wall -Wextra" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string (REPLACE "-pedantic" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE(Threads)
FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(PythonLibs REQUIRED)
pkg_check_modules(PC_COMMON REQUIRED CommonAPI)
pkg_check_modules(PC_COMMON_DBUS REQUIRED CommonAPI-DBus)

IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
ENDIF(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${STD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${CMAKE_SOURCE_DIR} 
    ${DLT_INCLUDE_DIRS}
    ${PC_COMMON_INCLUDE_DIRS}
	${PC_COMMON_DBUS_INCLUDE_DIRS}
    ${INCLUDE_FOLDER}
    ${PYTHON_INCLUDE_DIRS}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    "../../AudioManagerDaemon/include"
    "../include"
)

link_directories(
        ${LINK_DIRECTORIES}
        ${PC_COMMON_LIBRARY_DIRS}
		${PC_COMMON_DBUS_LIBRARY_DIRS}
)
   
file(GLOB CAPI_PLUGIN_INTERFACE_SRCS_CXX 
     "../../AudioManagerDaemon/src/CAmCommonAPIWrapper.cpp"
     "../../AudioManagerDaemon/src/CAmSocketHandler.cpp"
     "../../AudioManagerDaemon/src/CAmDltWrapper.cpp"
     "../src/*.cpp"  
     "CAmRoutingInterfaceCAPITests.cpp"
     "CAmTestCAPIWrapper.cpp"
     "CAmCommandSenderCommon.cpp"
     "CAmRoutingSenderService.cpp"
     "IAmRoutingSenderBackdoor.cpp"
)

file(GLOB_RECURSE  CAPI_PLUGIN_INTERFACE_GEN_SRCS_CXX 
     "../src-gen/*.cpp" 
)

set (CAPI_SENDER_INTERFACE "org.genivi.audiomanger.routingsender")
set (CAPI_SENDER_INSTANCE "org.genivi.routingsender")
set (CAPI_SENDER_PATH "/org/genivi/audiomanger/routingsender")

set (CAPI_SENDER_INTERFACE_H "\"${CAPI_SENDER_INTERFACE}\"")
set (CAPI_SENDER_INSTANCE_H "\"${CAPI_SENDER_INSTANCE}\"")
set (CAPI_SENDER_PATH_H "\"${CAPI_SENDER_PATH}\"")

set (CAPI_ROUTING_INTERFACE "org.genivi.audiomanger.routinginterface")
set (CAPI_ROUTING_INSTANCE "org.genivi.audiomanger")
set (CAPI_ROUTING_PATH "/org/genivi/audiomanger/routinginterface")

set (CAPI_ROUTING_INTERFACE_H "\"${CAPI_ROUTING_INTERFACE}\"")
set (CAPI_ROUTING_INSTANCE_H "\"${CAPI_ROUTING_INSTANCE}\"")
set (CAPI_ROUTING_PATH_H "\"${CAPI_ROUTING_PATH}\"")


ADD_EXECUTABLE(AmRoutingInterfaceCAPITests ${CAPI_PLUGIN_INTERFACE_SRCS_CXX} ${CAPI_PLUGIN_INTERFACE_GEN_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmRoutingInterfaceCAPITests 
    ${DLT_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PYTHON_LIBRARY}
    ${PC_GSTREAMER_LIBRARIES}
	${PC_COMMON_LIBRARIES}
	${PC_COMMON_DBUS_LIBRARIES}
    gtest
    gmock
)

add_custom_command(
    PRE_BUILD
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/CAmRoutingSenderService.h
    COMMAND generateheader ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CAmRoutingSenderService.h.in ${CMAKE_CURRENT_SOURCE_DIR}/CAmRoutingSenderService.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CAmRoutingSenderService.h.in
)

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RoutingSender_dbus.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/RoutingSender_dbus.conf)
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CAmRoutingSenderService.h.in ${CMAKE_CURRENT_SOURCE_DIR}/CAmRoutingSenderService.h)

FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/RoutingSender_dbus.conf ROUTING_TESTDOMAININTERFACE_DBUS_CONF)
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AmRoutingInterfaceCAPITests_dbus.conf.in ${EXECUTABLE_OUTPUT_PATH}/AmRoutingInterfaceCAPITests_dbus.conf)
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AmRoutingInterfaceCAPITests_dbus.conf.in ${EXECUTABLE_OUTPUT_PATH}/AmRoutingSenderTestDomain_dbus.conf)

INSTALL(TARGETS AmRoutingInterfaceCAPITests 
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

#execute_process(
#	    COMMAND cp "${CMAKE_SOURCE_DIR}/PluginCommandInterfaceCAPI/fidls/PluginCommandInterfaceCAPI_dbus.conf" ${EXECUTABLE_OUTPUT_PATH})

SET(ADD_DEPEND "audiomanager-bin" "sqlite3(>=3.6.22)" "dlt" "gtest"  "libpthread-stubs0")
set_property(GLOBAL APPEND PROPERTY tests_prop "${ADD_DEPEND}")

