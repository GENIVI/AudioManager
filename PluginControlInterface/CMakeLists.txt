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

PROJECT(PluginControlInterface)

set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/control)
set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/ControlPlugin)
set(INCLUDE_FOLDER "include")

FILE(READ "${AUDIO_INCLUDE_FOLDER}/control/IAmControlSend.h" VERSION_BUFFER LIMIT 6000)
STRING(REGEX MATCH "ControlSendVersion*.[^0-9]*[0-9].[0-9]*[0-9]" LIB_INTERFACE_VERSION_STRING ${VERSION_BUFFER})
STRING(REGEX REPLACE "ControlSendVersion*.." "" LIB_INTERFACE_VERSION ${LIB_INTERFACE_VERSION_STRING})
MESSAGE(STATUS "Building against control interface version ${LIB_INTERFACE_VERSION}")

FIND_PACKAGE(NSM REQUIRED)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
 	${CMAKE_CURRENT_BINARY_DIR}
 	${AUDIO_INCLUDE_FOLDER}
 	${PROJECT_INCLUDE_FOLDER}
 	${INCLUDE_FOLDER}
    ${NSM_INCLUDE_DIR}
)

# all source files go here
file(GLOB PLUGIN_CONTROL_SRCS_CXX "src/*.cpp")

add_library(PluginControlInterface SHARED ${PLUGIN_CONTROL_SRCS_CXX})

SET_TARGET_PROPERTIES(PluginControlInterface PROPERTIES 
                                            SOVERSION "${LIB_INTERFACE_VERSION}"
)

TARGET_LINK_LIBRARIES(PluginControlInterface 
    ${DLT_LIBRARIES})

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

#IF(WITH_DOCUMENTATION)
#	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
#	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
#  	add_custom_target (PluginControlInterfaceDBusDocs ALL 
#  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
#  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
#	)
#ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS PluginControlInterface 
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/control"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

SET(ADD_DEPEND "audiomanager-bin")
set_property(GLOBAL APPEND PROPERTY sampleplugins_prop "${ADD_DEPEND}")
