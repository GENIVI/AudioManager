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
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012,2015
#
# For further information see http://www.genivi.org/.
#


cmake_minimum_required(VERSION 2.8.8)

project(AudioManagerDeamon)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include ( CMakeDependentOption )
include ( MacroVersionFromGit )
include ( MacroInterfaceVersions )
include ( CMakePackageConfigHelpers )
include ( GNUInstallDirs )

if (NOT ${CMAKE_TOOLCHAIN_FILE} EQUAL "")
    message(STATUS "Using CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}'" )
endif (NOT ${CMAKE_TOOLCHAIN_FILE} EQUAL "")

find_package(PkgConfig)
find_package(CommonAPI)

option( WITH_TESTS
	"Build together with all available unitTest" ON )

option( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

option( WITH_DLT 
    "Enable automotive-DLT Support" ON )

option( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

option( WITH_TELNET
	"Build with Telnetserver (can only work with SocketHandler)" ON)
	
option ( WITH_SYSTEMD_WATCHDOG
    "Build with systemD support & watchdog" OFF)
	
option ( USE_BUILD_LIBS
	"Build with default library path = build path" ON)
	
option ( GLIB_DBUS_TYPES_TOLERANT
    "Build dbus with tolerance towards glib 16bit/32bit handling" ON)

option ( WITH_CAPI_WRAPPER
	"Build with commonapi wrapper" ON)
	
option ( WITH_DBUS_WRAPPER
	"Build with Dbus wrapper" OFF)	
	
option ( WITH_SHARED_UTILITIES
    "Build audio manager utilities as dynamic library" OFF)	
    
option ( WITH_DATABASE_STORAGE
    "Build with sqlite as in memory storage" OFF)    
     
set(DBUS_SERVICE_PREFIX "org.genivi.audiomanager" 
    CACHE PROPERTY "The dbus service prefix for the AM - only changable for legacy dbus")

set(DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager" 
    CACHE PROPERTY "The service object path of the AM - only changable for legacy dbus")

set(DEFAULT_TELNETPORT 6080 
    CACHE INTEGER "The telnet port that is offered by the AM, only valid if TELNET is ON")
	
set(MAX_TELNETCONNECTIONS 3 
    CACHE INTEGER "The maximum of allowed telnet connections, only valid if TELNET is ON")

set(DYNAMIC_ID_BOUNDARY 100 
    CACHE INTEGER "The ID boundary between static and dynamic ID ranges")

set(PLUGINS_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/plugins
    CACHE STRINGS "The output path of the plugins")

set(LIB_INSTALL_SUFFIX "audiomanager"
    CACHE STRINGS "The suffix used for installation of the plugins")
    
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin
    CACHE STRINGS "The binary will be placed here")    

set(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin/tests
    CACHE STRINGS "The test binaries will be placed here")
    
set(TEST_EXECUTABLE_INSTALL_PATH "~/tests"
    CACHE STRINGS "The test binaries will be installed here")    
    
set(DOC_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/doc
    CACHE STRINGS "The doxygen documentation will be placed here")  
    
set(AM_SHARE_FOLDER ${CMAKE_INSTALL_PREFIX}/share/audiomanager
    CACHE STRINGS "The share folder for the AM. Some DBus xmls will be placed here and so on")

set(AUDIOMANAGER_INCLUDE_FOLDER  ${CMAKE_BINARY_DIR}/include)
set(AUDIO_INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/include)
set(AUDIOMANAGER_UTILITIES ${CMAKE_SOURCE_DIR}/AudioManagerUtilities)
set(AUDIOMANAGER_UTILITIES_INCLUDE ${AUDIOMANAGER_UTILITIES}/include)
set(AUDIOMANAGER_UTILITIES_SRC ${AUDIOMANAGER_UTILITIES}/src)
set(DOXY_PROTOTYPE ${CMAKE_SOURCE_DIR}/cmake/DoxyFile.in)
set(DOXY_FILE ${CMAKE_CURRENT_BINARY_DIR}/DoxyFile)
set(AUDIOMANAGER_DAEMON_FOLDER ${CMAKE_SOURCE_DIR}/AudioManagerDaemon)
set(GOOGLE_MOCK_PROJECT_FOLDER ${CMAKE_SOURCE_DIR}/googleMock)
set(TCLAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/tclap-1.2.1/include)

GET_COMMAND_INTERFACE_VERSION(${AUDIO_INCLUDE_FOLDER})
GET_ROUTING_INTERFACE_VERSION(${AUDIO_INCLUDE_FOLDER})
GET_CONTROL_INTERFACE_VERSION(${AUDIO_INCLUDE_FOLDER})
message( STATUS "CommandInterface version: ${LIB_COMMAND_INTERFACE_VERSION}")
message( STATUS "ControlInterface version: ${LIB_CONTROL_INTERFACE_VERSION}")
message( STATUS "RoutingInterface version: ${LIB_ROUTING_INTERFACE_VERSION}")

if(USE_BUILD_LIBS)
	if(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
		set(DEFAULT_PLUGIN_COMMAND_DIR "${PLUGINS_OUTPUT_PATH}/command")
	endif(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)	
	if(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)	
		set(DEFAULT_PLUGIN_ROUTING_DIR "${PLUGINS_OUTPUT_PATH}/routing")
	endif(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)		
	if(NOT DEFINED CONTROLLER_PLUGIN)
		set(CONTROLLER_PLUGIN "${PLUGINS_OUTPUT_PATH}/control/libPluginControlInterface.so")
	endif(NOT DEFINED CONTROLLER_PLUGIN)
else(USE_BUILD_LIBS)
	if(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
		set(DEFAULT_PLUGIN_COMMAND_DIR "${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX}/command")
	endif(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
	if(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)
		set(DEFAULT_PLUGIN_ROUTING_DIR "${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX}/routing")
	endif(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)
	if(NOT DEFINED CONTROLLER_PLUGIN)
		set(CONTROLLER_PLUGIN "${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX}/control/libPluginControlInterface.so")
	endif(NOT DEFINED CONTROLLER_PLUGIN)
endif(USE_BUILD_LIBS)


if(USE_BUILD_LIBS) 
     execute_process(COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif(USE_BUILD_LIBS)

if(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
    add_definitions(${DLT_CFLAGS_OTHER})
    include_directories(${DLT_INCLUDE_DIRS})
    link_directories(${DLT_LIBRARY_DIRS})
endif(WITH_DLT)

if(WITH_TESTS)	
	add_subdirectory(googleMock)
	set(GMOCK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/googleMock/include")
	set(GOOGLE_TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/googleMock/gtest/include")
endif(WITH_TESTS)

##global build flags set(CPACK_RPM_COMPONENT_INSTALL ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -pedantic -rdynamic -Wno-variadic-macros")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

configure_package_config_file ( 
    ${CMAKE_SOURCE_DIR}/cmake/audiomanagerConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/audiomanagerConfig.cmake
    INSTALL_DESTINATION lib/${LIB_INSTALL_SUFFIX}/cmake
    PATH_VARS AUDIO_INCLUDE_FOLDER )
                              
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AudioManagerVersion.cmake
    VERSION ${DAEMONVERSION}
    COMPATIBILITY SameMajorVersion )
 
add_subdirectory (AudioManagerDaemon)
add_subdirectory (AudioManagerUtilities)

if(EXISTS "${CMAKE_SOURCE_DIR}/Plugins/")
    add_subdirectory (${CMAKE_SOURCE_DIR}/Plugins)
endif(EXISTS "${CMAKE_SOURCE_DIR}/Plugins/")                         
   
configure_file( ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/docx/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/DoxyFile )
configure_file( ${CMAKE_SOURCE_DIR}/cmake/config.cmake.in ${CMAKE_BINARY_DIR}/include/audiomanagerconfig.h )
configure_file( ${CMAKE_SOURCE_DIR}/cmake/audiomanager.pc.in ${CMAKE_BINARY_DIR}/audiomanager.pc @ONLY )

install(FILES ${CMAKE_BINARY_DIR}/audiomanager.pc DESTINATION lib/pkgconfig COMPONENT devel)    
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/audiomanagerConfig.cmake 
    DESTINATION lib/${LIB_INSTALL_SUFFIX}/cmake
    COMPONENT dev)    
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/audiomanagerconfig.h  
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIB_INSTALL_SUFFIX}
    COMPONENT dev)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/tclap-1.2.1/include/tclap  
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${LIB_INSTALL_SUFFIX}
    COMPONENT dev)    

 
if(WITH_DOCUMENTATION)
	find_package(Doxygen)
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
	configure_file("README.html" ${DOC_OUTPUT_PATH}/html/README.html)
  	add_custom_target (AudioManangerDoku ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  		SOURCES ${CMAKE_SOURCE_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
	)
	install (DIRECTORY ${DOC_OUTPUT_PATH}/
		DESTINATION "${CMAKE_INSTALL_DOCDIR}"
		PATTERN "Doxyfile" EXCLUDE
		PATTERN "def" EXCLUDE)
endif(WITH_DOCUMENTATION)

SET(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")

add_custom_target(bin-install
  COMMAND
     "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=bin 
     -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

add_custom_target(dev-install
  COMMAND
     "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=dev 
     -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

add_custom_target(test-install
  COMMAND
     "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=tests
     -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

