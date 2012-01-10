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

PROJECT(PluginRoutingInterfaceAsync)

set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/RoutingAsync)
set(INCLUDES_FOLDER "include")

FILE(READ "${AUDIO_INCLUDES_FOLDER}/routing/RoutingSendInterface.h" VERSION_BUFFER LIMIT 6000)
STRING(REGEX MATCH "RoutingSendVersion*.[^0-9]*[0-9]" LIB_INTERFACE_VERSION_STRING ${VERSION_BUFFER})
STRING(REGEX REPLACE "[^0-9]" "" LIB_INTERFACE_VERSION ${LIB_INTERFACE_VERSION_STRING})
MESSAGE(STATUS "Building against routing interface version ${LIB_INTERFACE_VERSION}")

find_package( Threads )
FIND_PACKAGE(DBUS REQUIRED)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
 	${CMAKE_CURRENT_BINARY_DIR}
	${AUDIO_INCLUDES_FOLDER}
	${INCLUDES_FOLDER}
	${DBUS_INCLUDE_DIR}
    ${DBUS_ARCH_INCLUDE_DIR}
)

# all source files go here
file(GLOB PLUGINDBUS_SRCS_CXX "src/*.cpp")


add_library(PluginRoutingInterfaceAsync SHARED ${PLUGINDBUS_SRCS_CXX})

SET_TARGET_PROPERTIES(PluginRoutingInterfaceAsync PROPERTIES 
                                            SOVERSION "${LIB_INTERFACE_VERSION}"
)

TARGET_LINK_LIBRARIES(PluginRoutingInterfaceAsync
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

IF(WITH_DOCUMENTATION)
	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (PluginRoutingInterfaceAsyncDocs ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
	)
ENDIF(WITH_DOCUMENTATION)



