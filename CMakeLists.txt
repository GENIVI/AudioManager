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

#Can be changed via passing -DDBUS_SERVICE_PREFIX="XXX" to cmake
IF(NOT DEFINED DBUS_SERVICE_PREFIX)
	SET( DBUS_SERVICE_PREFIX "org.genivi.audiomanager\0" )
ENDIF(NOT DEFINED DBUS_SERVICE_PREFIX)

#Can be changed via passing -DDBUS_SERVICE_OBJECT_PATH="XXX" to cmake
IF(NOT DEFINED DBUS_SERVICE_OBJECT_PATH)
	SET( DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager\0" )
ENDIF(NOT DEFINED DBUS_SERVICE_OBJECT_PATH)

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
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++98")

if(WITH_PLUGIN_COMMAND)
	add_subdirectory (PluginCommandInterfaceDbus)
endif(WITH_PLUGIN_COMMAND)

if(WITH_PLUGIN_ROUTING)
	add_subdirectory (PluginRoutingInterfaceDbus)
endif(WITH_PLUGIN_ROUTING)

if(WITH_PLUGIN_CONTROL)
	add_subdirectory (PluginControlInterface)
endif(WITH_PLUGIN_CONTROL)

if(WITH_MAIN)
	add_subdirectory (AudioManagerDaemon)
endif(WITH_MAIN)




