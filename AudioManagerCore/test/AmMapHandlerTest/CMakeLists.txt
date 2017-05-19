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

project(AmMapHandlerTest LANGUAGES CXX VERSION ${DAEMONVERSION})

INCLUDE_DIRECTORIES(   
    ${AUDIOMANAGER_UTILITIES_INCLUDE}
    ${GMOCK_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

file(GLOB DATABASE_SRCS_CXX 
    "../CAmCommonFunctions.cpp" 
    "CAmTestDatabaseObserver.cpp"
    "*.cpp"
    )
    
ADD_EXECUTABLE( AmMapHandlerTest ${DATABASE_SRCS_CXX})

TARGET_LINK_LIBRARIES( AmMapHandlerTest 
        ${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
    	AudioManagerCore
)

ADD_TEST(AmMapHandlerTest AmMapHandlerTest)

ADD_DEPENDENCIES(AmMapHandlerTest AudioManagerCore)

INSTALL(TARGETS AmMapHandlerTest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

