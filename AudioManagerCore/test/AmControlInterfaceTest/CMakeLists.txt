# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. if a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 3.0)

project(AmControlInterfaceTest LANGUAGES CXX VERSION ${DAEMONVERSION})

INCLUDE_DIRECTORIES(  
    ${AUDIOMANAGER_CORE_INCLUDE} 
    ${GMOCK_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS})

    
 file(GLOB CONTROL_INTERFACE_SRCS_CXX 
    "../CAmCommonFunctions.cpp" 
    "*.cpp"    
)

LINK_DIRECTORIES(${AUDIO_MANAGER_CORE_LIB_DIRECTORIES})

add_executable(AmControlInterfaceTest ${CONTROL_INTERFACE_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmControlInterfaceTest 
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
	AudioManagerCore
)

add_test(AmControlInterfaceTest AmControlInterfaceTest)

ADD_DEPENDENCIES(AmControlInterfaceTest AudioManagerCore)

INSTALL(TARGETS AmControlInterfaceTest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests)




