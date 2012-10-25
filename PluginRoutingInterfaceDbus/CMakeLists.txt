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

PROJECT(PluginRoutingInterfaceDbus)

set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/RoutingPlugin)
set(INCLUDE_FOLDER "include")

SET(SHARED_FOLDER "${CMAKE_INSTALL_PREFIX}/share")
FILE(READ "${AUDIO_INCLUDE_FOLDER}/routing/IAmRoutingSend.h" VERSION_BUFFER LIMIT 6000)
STRING(REGEX MATCH "RoutingSendVersion*.[^0-9]*[0-9].[0-9]*[0-9]" LIB_INTERFACE_VERSION_STRING ${VERSION_BUFFER})
STRING(REGEX REPLACE "RoutingSendVersion*.." "" LIB_INTERFACE_VERSION ${LIB_INTERFACE_VERSION_STRING})
MESSAGE(STATUS "Building against routing interface version ${LIB_INTERFACE_VERSION}")

IF(USE_BUILD_LIBS) 
    SET(ROUTING_DBUS_INTROSPECTION_FOLDER ${EXECUTABLE_OUTPUT_PATH})
    SET(ROUTING_DBUS_INTROSPECTION_FILE ${EXECUTABLE_OUTPUT_PATH}/RoutingReceiver.xml)
ELSE(USE_BUILD_LIBS) 
    SET(ROUTING_DBUS_INTROSPECTION_FOLDER ${SHARED_FOLDER}/audiomanager)
    SET(ROUTING_DBUS_INTROSPECTION_FILE ${SHARED_FOLDER}/audiomanager/RoutingReceiver.xml)
ENDIF(USE_BUILD_LIBS)


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake ${CMAKE_CURRENT_SOURCE_DIR}/include/configRoutingDbus.h )

FIND_PACKAGE(DBUS REQUIRED)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
 	${CMAKE_CURRENT_BINARY_DIR}
 	${DBUS_INCLUDE_DIR}
	${DBUS_ARCH_INCLUDE_DIR}
	${AUDIO_INCLUDE_FOLDER}
	${PROJECT_INCLUDE_FOLDER}
	${INCLUDE_FOLDER}
)

# all source files go here
file(GLOB PLUGINDBUS_SRCS_CXX "src/*.cpp")


add_library(PluginRoutingInterfaceDbus SHARED ${PLUGINDBUS_SRCS_CXX})

SET_TARGET_PROPERTIES(PluginRoutingInterfaceDbus PROPERTIES 
                                            SOVERSION "${LIB_INTERFACE_VERSION}"
)

TARGET_LINK_LIBRARIES(PluginRoutingInterfaceDbus 
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
)

IF(WITH_TESTS)
#	add_subdirectory (test)
ENDIF(WITH_TESTS)

IF(USE_BUILD_LIBS) 
    execute_process(
    		COMMAND mkdir -p "${CMAKE_SOURCE_DIR}/bin"
    		COMMAND cp "${CMAKE_SOURCE_DIR}/PluginRoutingInterfaceDbus/${INCLUDE_FOLDER}/RoutingReceiver.xml" ${EXECUTABLE_OUTPUT_PATH}/RoutingReceiver.xml)
    execute_process(
    		COMMAND mkdir -p "${CMAKE_SOURCE_DIR}/bin"
    		COMMAND cp "${CMAKE_SOURCE_DIR}/PluginRoutingInterfaceDbus/${INCLUDE_FOLDER}/RoutingSender.xml" ${EXECUTABLE_OUTPUT_PATH}/RoutingSender.xml)
ENDIF(USE_BUILD_LIBS) 


#IF(WITH_DOCUMENTATION)
#	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
#	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
#  	add_custom_target (PluginRoutingInterfaceDBusDocs ALL 
#  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
#  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
#	)
#ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS PluginRoutingInterfaceDbus 
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/routing"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/RoutingReceiver.xml 
        DESTINATION "${ROUTING_DBUS_INTROSPECTION_FOLDER}"
        COMPONENT sampleplugins
)


SET(ADD_DEPEND "audiomanager-bin" "dlt" "libdbus-1-3(>=1.2.16)")
set_property(GLOBAL APPEND PROPERTY sampleplugins_prop "${ADD_DEPEND}")

