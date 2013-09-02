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

PROJECT(CAmCommandSenderCAPITests)

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
     "CAmCommandSenderCAPITest.cpp"
     "CAmTestCAPIWrapper.cpp"
     "CAmCommandSenderCommon.cpp"
)

file(GLOB_RECURSE  CAPI_PLUGIN_INTERFACE_GEN_SRCS_CXX 
     "../src-gen/*.cpp" 
)

ADD_EXECUTABLE(AmCommandSenderCAPITest ${CAPI_PLUGIN_INTERFACE_SRCS_CXX} ${CAPI_PLUGIN_INTERFACE_GEN_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmCommandSenderCAPITest 
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PYTHON_LIBRARY}
    ${PC_GSTREAMER_LIBRARIES}
	${PC_COMMON_LIBRARIES}
	${PC_COMMON_DBUS_LIBRARIES}
    gtest
    gmock
)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/PluginCommandInterfaceCAPI/fidl/PluginCommandInterfaceCAPI_dbus.conf ${EXECUTABLE_OUTPUT_PATH}/AmCommandSenderCAPITest_dbus.conf)

INSTALL(TARGETS AmCommandSenderCAPITest 
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

#execute_process(
#	    COMMAND cp "${CMAKE_SOURCE_DIR}/PluginCommandInterfaceCAPI/fidls/PluginCommandInterfaceCAPI_dbus.conf" ${EXECUTABLE_OUTPUT_PATH})

SET(ADD_DEPEND "audiomanager-bin" "sqlite3(>=3.6.22)" "dlt" "gtest"  "libpthread-stubs0")
set_property(GLOBAL APPEND PROPERTY tests_prop "${ADD_DEPEND}")

