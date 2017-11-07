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

project (AudioManagerDeamon LANGUAGES CXX VERSION ${DAEMONVERSION})

set(AUDIOMAN_SRCS_CXX
    src/main.cpp)
    
INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}	
	${AUDIO_INCLUDE_FOLDER}
	${AUDIOMANAGER_INCLUDE_FOLDER}
	${AUDIOMANAGER_UTILITIES_INCLUDE}
	${AUDIOMANAGER_CORE_INCLUDE}
	${INCLUDE_FOLDER}
	${TCLAP_INCLUDE_DIR}
)

LINK_DIRECTORIES(${AUDIO_MANAGER_CORE_LIB_DIRECTORIES})

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX})

TARGET_LINK_LIBRARIES(AudioManager AudioManagerCore ${AUDIO_MANAGER_CORE_LIBS})

ADD_DEPENDENCIES(AudioManager AudioManagerCore)

INSTALL(TARGETS AudioManager
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT bin)

INSTALL(DIRECTORY "${AUDIO_INCLUDE_FOLDER}/" 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_INSTALL_SUFFIX}
        COMPONENT dev)


