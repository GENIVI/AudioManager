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

PROJECT(PluginRoutingInterfaceCAPI)

OPTION(WITH_ROUTING_INTERFACE_CAPI
       "build with routing iterface common-api plugin" ON)

IF(WITH_ROUTING_INTERFACE_CAPI AND WITH_CAPI_WRAPPER)

set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/RoutingPlugin)
set(INCLUDE_FOLDER "include")

SET(SHARED_FOLDER "${CMAKE_INSTALL_PREFIX}/share")
FILE(READ "${AUDIO_INCLUDE_FOLDER}/routing/IAmRoutingSend.h" VERSION_BUFFER LIMIT 6000)
STRING(REGEX MATCH "RoutingSendVersion*.[^0-9]*[0-9].[0-9]*[0-9]" LIB_INTERFACE_VERSION_STRING ${VERSION_BUFFER})
STRING(REGEX REPLACE "RoutingSendVersion*.." "" LIB_INTERFACE_VERSION ${LIB_INTERFACE_VERSION_STRING})
MESSAGE(STATUS "Building against routing interface version ${LIB_INTERFACE_VERSION}")

INSERT_DBUS_CONF_IF_NEEDED("@ROUTING_INTERFACE_DBUS_CONF@" 
                          "${CMAKE_CURRENT_SOURCE_DIR}/fidl/PluginRoutingInterfaceCAPI_dbus.conf" 
                          ROUTING_INTERFACE_DBUS_CONF)

set(ROUTING_INTERFACE_DBUS_CONF ${ROUTING_INTERFACE_DBUS_CONF} PARENT_SCOPE)


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake ${CMAKE_CURRENT_SOURCE_DIR}/include/configRoutingCAPI.h )

FIND_PACKAGE(CommonAPI REQUIRED)
FIND_PACKAGE(CommonAPI_DBus REQUIRED) 

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
 	${CMAKE_CURRENT_BINARY_DIR}
	${AUDIO_INCLUDE_FOLDER}
	${PROJECT_INCLUDE_FOLDER}
	${COMMON_API_INCLUDE_DIRS}
	${COMMON_API_DBUS_INCLUDE_DIRS}
	${COMMON_API_GEN_INCLUDE_DIR}
	${INCLUDE_FOLDER}
)

# all source files go here
file(GLOB PLUGINCAPI_SRCS_CXX "src/*.cpp" ${COMMON_API_GEN_SOURCES})


add_library(PluginRoutingInterfaceCAPI MODULE ${PLUGINCAPI_SRCS_CXX})

SET_TARGET_PROPERTIES(PluginRoutingInterfaceCAPI PROPERTIES 
                                            SOVERSION "${LIB_INTERFACE_VERSION}"
)

TARGET_LINK_LIBRARIES(PluginRoutingInterfaceCAPI 
    ${DLT_LIBRARIES}
    ${COMMON_API_LIBRARIES}
    ${COMMON_API_DBUS_LIBRARIES}
)

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

#IF(WITH_DOCUMENTATION)
#	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
#	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
#  	add_custom_target (PluginRoutingInterfaceCAPIDocs ALL 
#  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
#  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
#	)
#ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS PluginRoutingInterfaceCAPI 
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/routing"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

SET(ADD_DEPEND "audiomanager-bin" "dlt")
set_property(GLOBAL APPEND PROPERTY sampleplugins_prop "${ADD_DEPEND}")

ENDIF(WITH_ROUTING_INTERFACE_CAPI AND WITH_CAPI_WRAPPER)
