# Copyright (c) 2012 GENIVI Alliance
# Copyright (c) 2012 BMW
#
# author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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

PROJECT(AmControlReceiverShadowTest)

set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE (Threads)
FIND_PACKAGE(PkgConfig)


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
    ${INCLUDE_FOLDER}
    ${DBUS_INCLUDE_DIR}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    "../../AudioManagerDaemon/include"
    "../include"
)
   
file(GLOB CONTROL_RECEIVER_PLUGIN_INTERFACE_SRCS_CXX 
     "../../AudioManagerDaemon/src/CAmSocketHandler.cpp"
     "../../AudioManagerDaemon/src/CAmDltWrapper.cpp"  
     "../../AudioManagerDaemon/src/CAmDatabaseObserver.cpp"  
     "../../AudioManagerDaemon/src/CAmCommandSender.cpp"
     "../../AudioManagerDaemon/src/CAmTelnetServer.cpp" 
     "../../AudioManagerDaemon/src/CAmTelnetMenuHelper.cpp"
     "../../AudioManagerDaemon/src/CAmRoutingSender.cpp"
     "../../AudioManagerDaemon/src/CAmRoutingReceiver.cpp"
     "../../AudioManagerDaemon/src/CAmCommandReceiver.cpp"
     "../../AudioManagerDaemon/src/CAmControlSender.cpp"
     "../../AudioManagerDaemon/src/CAmControlReceiver.cpp"
     "../../AudioManagerDaemon/src/CAmRouter.cpp"
     "../src/IAmControlReceiverShadow.cpp"  
     "CAmControlReceiverShadowTest.cpp" 
)

IF(WITH_DATABASE_STORAGE)
    SET (CONTROL_RECEIVER_PLUGIN_INTERFACE_SRCS_CXX
		${CONTROL_RECEIVER_PLUGIN_INTERFACE_SRCS_CXX}
		"../../AudioManagerDaemon/src/CAmDatabaseHandlerSQLite.cpp")
ELSE(WITH_DATABASE_STORAGE)
    SET (CONTROL_RECEIVER_PLUGIN_INTERFACE_SRCS_CXX
		${CONTROL_RECEIVER_PLUGIN_INTERFACE_SRCS_CXX}
		"../../AudioManagerDaemon/src/CAmDatabaseHandlerMap.cpp")
ENDIF(WITH_DATABASE_STORAGE)

ADD_EXECUTABLE(AmControlReceiverShadowTest ${CONTROL_RECEIVER_PLUGIN_INTERFACE_SRCS_CXX})


TARGET_LINK_LIBRARIES(AmControlReceiverShadowTest 
    ${DLT_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${DBUS_LIBRARIES}
    ${SQLITE_LIBRARIES}
    gtest
    gmock
    rt
)

ADD_DEPENDENCIES(AmControlReceiverShadowTest gtest gmock)

INSTALL(TARGETS AmControlReceiverShadowTest 
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

SET(ADD_DEPEND "audiomanager-bin" "dlt" "libdbus-1-3(>=1.2.16)"  "libpthread-stubs0")
set_property(GLOBAL APPEND PROPERTY tests_prop "${ADD_DEPEND}")

