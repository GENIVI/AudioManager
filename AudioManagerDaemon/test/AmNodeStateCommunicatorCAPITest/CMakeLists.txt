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

cmake_minimum_required(VERSION 2.8.8)

project(AmNodeStateCommunicatorCAPITest)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DUNIT_TEST=1")

find_package(Threads)
find_package(PythonLibs REQUIRED)

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
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${PYTHON_INCLUDE_DIRS}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    ${COMMONAPI_DBUS_INCLUDE_DIRS}
    ${COMMONAPI_INCLUDE_DIRS}
	${COMMON_API_GEN_INCLUDE_DIR}
    ${DBUS_INCLUDE_DIRS}
)

if(WITH_DLT)    
INCLUDE_DIRECTORIES(
    ${INCLUDE_DIRECTORIES} 
    ${DLT_INCLUDE_DIRS})
endif(WITH_DLT) 

  
file(GLOB NODESTATECOMMUNICATORCAPI_SRCS_CXX 
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmDatabaseHandler.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmDatabaseObserver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRoutingSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRoutingReceiver.cpp"    
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmNodeStateCommunicatorCAPI.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmControlSender.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmCommandSender.cpp"  
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmCommandReceiver.cpp"
    "${AUDIOMANAGER_DAEMON_FOLDER}/src/CAmRouter.cpp"   
     "${AUDIOMANAGER_UTILITIES_SRC}/CAmCommonAPIWrapper.cpp"
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmDltWrapper.cpp"
    "${AUDIOMANAGER_UTILITIES_SRC}/CAmSocketHandler.cpp"
    "../CAmCommonFunctions.cpp"     
    "*.cpp")

ADD_EXECUTABLE(AmNodeStateCommunicatorCAPITest ${NODESTATECOMMUNICATORCAPI_SRCS_CXX} ${COMMON_API_GEN_SOURCES})

TARGET_LINK_LIBRARIES(AmNodeStateCommunicatorCAPITest 
    ${AM_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PYTHON_LIBRARY}
    ${CommonAPI-DBus_LIBRARY}
    ${CommonAPI_LIBRARY}
    ${DBUS_LIBRARIES}
    gtest
    gmock
)

ADD_DEPENDENCIES(AmNodeStateCommunicatorCAPITest gtest gmock)

INSTALL(TARGETS AmNodeStateCommunicatorCAPITest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

INSTALL(FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/org.genivi.NodeStateManager.Consumer.xml 
        ${CMAKE_CURRENT_SOURCE_DIR}/nsm.py
        ${CMAKE_CURRENT_SOURCE_DIR}/send2nsm.py
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

if(USE_BUILD_LIBS) 
        execute_process(
	    COMMAND mkdir -p ${EXECUTABLE_OUTPUT_PATH}
	    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/org.genivi.NodeStateManager.Consumer.xml" ${EXECUTABLE_OUTPUT_PATH}/org.genivi.NodeStateManager.Consumer.xml
	    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/nsm.py" ${EXECUTABLE_OUTPUT_PATH}/nsm.py
	    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/send2nsm.py" ${EXECUTABLE_OUTPUT_PATH}/send2nsm.py
#	    COMMAND cp "${AUDIOMANAGER_DAEMON_FOLDER}/fidls/NodeStateManager_dbus.conf" ${EXECUTABLE_OUTPUT_PATH}/AmNodeStateCommunicatorCAPITest_dbus.conf
        )
endif(USE_BUILD_LIBS) 

