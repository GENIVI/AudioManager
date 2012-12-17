# Copyright (c) 2012 GENIVI Alliance
# Copyright (c) 2012 BMW
#
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012
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

PROJECT(AmNodeStateCommunicatorTest)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DUNIT_TEST=1")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE(Threads)
FIND_PACKAGE(DBUS REQUIRED)
FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(PythonLibs REQUIRED)
pkg_check_modules(SQLITE REQUIRED sqlite3)


IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
ENDIF(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${STD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_INCLUDE_FOLDER} 
    ${CMAKE_SOURCE_DIR} 
    ${DLT_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIR}
    ${INCLUDE_FOLDER}
    ${PYTHON_INCLUDE_DIRS}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    "../../include"
)
   
file(GLOB NODESTATECOMMUNICATOR 
    "../../src/CAmDatabaseHandler.cpp"
    "../../src/CAmDatabaseObserver.cpp"
    "../../src/CAmRoutingSender.cpp"
    "../../src/CAmRoutingReceiver.cpp"    
     "../../src/CAmDbusWrapper.cpp"
     "../../src/CAmDltWrapper.cpp"
     "../../src/CAmSocketHandler.cpp"
     "../../src/CAmNodeStateCommunicator.cpp"
     "../../src/CAmControlSender.cpp"
     "../../src/CAmCommandSender.cpp"  
     "../../src/CAmCommandReceiver.cpp"
     "../CAmCommonFunctions.cpp" 
    "../../src/CAmRouter.cpp"     
     "*.cpp"  
     )

ADD_EXECUTABLE(AmNodeStateCommunicatorTest ${NODESTATECOMMUNICATOR})

TARGET_LINK_LIBRARIES(AmNodeStateCommunicatorTest 
    ${SQLITE_LIBRARIES}
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PYTHON_LIBRARY}
    gtest
    gmock
)

ADD_DEPENDENCIES(AmNodeStateCommunicatorTest gtest gmock)

INSTALL(TARGETS AmNodeStateCommunicatorTest 
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

INSTALL(FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/org.genivi.NodeStateManager.Consumer.xml 
        ${CMAKE_CURRENT_SOURCE_DIR}/nsm.py
        ${CMAKE_CURRENT_SOURCE_DIR}/send2nsm.py
        DESTINATION "~/AudioManagerTest/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

IF(USE_BUILD_LIBS) 
        execute_process(
	    COMMAND mkdir -p "${EXECUTABLE_OUTPUT_PATH}"
	    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/org.genivi.NodeStateManager.Consumer.xml" ${EXECUTABLE_OUTPUT_PATH}/org.genivi.NodeStateManager.Consumer.xml
	    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/nsm.py" ${EXECUTABLE_OUTPUT_PATH}/nsm.py
	    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/send2nsm.py" ${EXECUTABLE_OUTPUT_PATH}/send2nsm.py)
ENDIF(USE_BUILD_LIBS) 

SET(ADD_DEPEND "audiomanager-bin" "sqlite3(>=3.6.22)" "dlt" "libdbus-1-3(>=1.2.16)" "gtest"  "libpthread-stubs0")
set_property(GLOBAL APPEND PROPERTY tests_prop "${ADD_DEPEND}")

