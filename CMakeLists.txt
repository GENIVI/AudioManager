 #
 #  Copyright (C) 2011, BMW AG 
 # 
 # AudioManagerDeamon
 #
 # @file CMakeLists.txt
 #
 # @date: 20.05.2011
 # @author: Christian Müller (christian.ei.mueller@bmw.de)
 #

cmake_minimum_required(VERSION 2.6)

execute_process(COMMAND git describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE DAEMONVERSION 
                OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "Build Version ${DAEMONVERSION}")

execute_process(COMMAND git log --pretty=short WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG)

PROJECT(AudioManagerDeamon)

OPTION( WITH_DBUS_WRAPPER 
	"Enable Dbus Wrapper Support" ON )

OPTION( WITH_SOCKETHANDLER_LOOP 
	"Enable SocketHandler Support (cannot work with SimpleDBusLoop)" ON )

OPTION( WITH_SIMPLEDBUS_LOOP 
	"Enable SimpleDBusLoop (requires DBus_Wrapper,cannot work with SocketHandler)" OFF )

OPTION( WITH_TESTS
	"Build together with all available unitTest" ON )

OPTION( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

OPTION( WITH_PLUGIN_COMMAND
	"Build command pluings" ON)

OPTION( WITH_PLUGIN_CONTROL
	"Build control pluing" ON)

OPTION( WITH_PLUGIN_ROUTING
	"Build routing pluings" ON)

OPTION( WITH_MAIN
	"Build with daemon" ON)

OPTION( WITH_PPOLL
	"libc supports ppoll" ON)

OPTION( WITH_TELNET
	"build with Telnetserver" OFF)

#Can be changed via passing -DDBUS_SERVICE_PREFIX="XXX" to cmake
IF(NOT DEFINED DBUS_SERVICE_PREFIX)
	SET( DBUS_SERVICE_PREFIX "org.genivi.audiomanager\0" )
ENDIF(NOT DEFINED DBUS_SERVICE_PREFIX)

#Can be changed via passing -DDBUS_SERVICE_OBJECT_PATH="XXX" to cmake
IF(NOT DEFINED DBUS_SERVICE_OBJECT_PATH)
	SET( DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager\0" )
ENDIF(NOT DEFINED DBUS_SERVICE_OBJECT_PATH)

#Can be changed via passing -DDEFAULT_TELNETPORT="XXX" to cmake
IF(NOT DEFINED DEFAULT_TELNETPORT)
	SET( DEFAULT_TELNETPORT 6060 )
ENDIF(NOT DEFINED DEFAULT_TELNETPORT)

#Can be changed via passing -DMAX_TELNETCONNECTIONS="XXX" to cmake
IF(NOT DEFINED MAX_TELNETCONNECTIONS)
	SET( MAX_TELNETCONNECTIONS 3 )
ENDIF(NOT DEFINED MAX_TELNETCONNECTIONS)

SET(DLT_CONTEXT AudioManager)
SET(PLUGINS_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/plugins)

SET(DEFAULT_PLUGIN_COMMAND_DIR "${PLUGINS_OUTPUT_PATH}/command")
SET(DEFAULT_PLUGIN_ROUTING_DIR "${PLUGINS_OUTPUT_PATH}/routing")
SET(CONTROLLER_PLUGIN "${PLUGINS_OUTPUT_PATH}/control/libPluginControlInterface.so")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
SET(AUDIO_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/includes)
SET(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/doc)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/test)
SET(DOXY_FILE ${CMAKE_SOURCE_DIR}/cmake/DoxyFile.in)
SET(INTROSPECTION_COMMAND_XML_FILE "${AUDIO_INCLUDES_FOLDER}/dbus/CommandInterface.xml")

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/cmake/config.cmake ${CMAKE_SOURCE_DIR}/includes/config.h )

FIND_PACKAGE(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)

IF(WITH_DOCUMENTATION)
	find_package(Doxygen)
	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH}/Interfaces)
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Interfaces/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Interfaces ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Interfaces/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}/Interfaces
  		SOURCES ${AUDIO_INCLUDES_FOLDER} ${DOC_OUTPUT_PATH}/Interfaces/Doxyfile
	)
ENDIF(WITH_DOCUMENTATION)

##global build flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=c++98 -D_GNU_SOURCE")

if(WITH_PLUGIN_COMMAND)
	add_subdirectory (PluginCommandInterfaceDbus)
endif(WITH_PLUGIN_COMMAND)

if(WITH_PLUGIN_ROUTING)
	add_subdirectory (PluginRoutingInterfaceDbus)
	add_subdirectory (PluginRoutingInterfaceAsync)
endif(WITH_PLUGIN_ROUTING)

if(WITH_PLUGIN_CONTROL)
	add_subdirectory (PluginControlInterface)
endif(WITH_PLUGIN_CONTROL)

if(WITH_MAIN)
	add_subdirectory (AudioManagerDaemon)
endif(WITH_MAIN)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)





