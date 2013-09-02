# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#


cmake_minimum_required(VERSION 2.6)
include(CMakeDependentOption)

execute_process(COMMAND git describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE DAEMONVERSION 
                OUTPUT_STRIP_TRAILING_WHITESPACE)           

IF (NOT DAEMONVERSION)
    #Can be changed via passing -DVERSION="XXX" to cmake
    IF(NOT DEFINED VERSION)
        SET( DAEMONVERSION "homebrew-${CMAKE_SOURCE_DIR}" )
    ELSE (NOT DEFINED VERSION)
        SET( DAEMONVERSION "${VERSION}" )   
    ENDIF(NOT DEFINED VERSION)
ELSE (NOT DAEMONVERSION)
    STRING(REGEX REPLACE "(-)[^-]+$" "" DAEMONVERSION ${DAEMONVERSION})
    STRING(REGEX REPLACE "-" "." DAEMONVERSION ${DAEMONVERSION})
ENDIF(NOT DAEMONVERSION)

message(STATUS "Build Version ${DAEMONVERSION}")

execute_process(COMMAND git log --pretty=short WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG)

PROJECT(AudioManagerDeamon)

FIND_PACKAGE(PkgConfig)

SET(WITH_ENABLED_IPC "CAPI" CACHE STRING "Disable 'NONE' / Enable Common-API 'CAPI' or Dbus 'DBUS' Support")

SET_PROPERTY(CACHE WITH_ENABLED_IPC PROPERTY STRINGS "NONE" "CAPI" "DBUS") 

OPTION( WITH_DLT 
    "Enable automotive-DLT Support" ON )

OPTION( WITH_TESTS
	"Build together with all available unitTest" ON )

OPTION( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

OPTION( WITH_PLUGIN_COMMAND
	"Build command pluings" ON)

OPTION( WITH_PLUGIN_CONTROL
	"Build control plugin" ON)

OPTION( WITH_PLUGIN_ROUTING
	"Build routing pluings" ON)

OPTION( WITH_TELNET
	"build with Telnetserver (can only work with SocketHandler)" ON)
	
OPTION ( WITH_SYSTEMD_WATCHDOG
    "build with systemD support & watchdog" OFF)
	
OPTION ( USE_BUILD_LIBS
	"build with default library path = build path" ON)
	
OPTION ( GLIB_DBUS_TYPES_TOLERANT
    "build dbus with tolerance towards glib 16bit/32bit handling" ON)

IF (WITH_ENABLED_IPC STREQUAL "NONE")
   SET (ENABLE_NSM OFF)
ELSE  ()
   SET (ENABLE_NSM ON)
ENDIF ()

CMAKE_DEPENDENT_OPTION(WITH_NSM "build with NSM support" ON
                        "ENABLE_NSM" OFF)

OPTION ( WITH_NSM
    "build with NSM support" ON)
  
 OPTION ( WITH_DATABASE_STORAGE
    "build with sqlite as in memory storage" OFF)
 
IF (WITH_ENABLED_IPC STREQUAL "DBUS")
    SET (WITH_DBUS_WRAPPER ON CACHE INTERNAL "hide this!" FORCE)
    SET (WITH_CAPI_WRAPPER OFF CACHE INTERNAL "hide this!" FORCE)
ELSEIF(WITH_ENABLED_IPC STREQUAL "CAPI")
    SET (WITH_CAPI_WRAPPER ON CACHE INTERNAL "hide this!" FORCE)
    SET (WITH_DBUS_WRAPPER OFF CACHE INTERNAL "hide this!" FORCE)
    
    MACRO(INSERT_DBUS_CONF_IF_NEEDED IN_PLACEHOLDER IN_SRC_DBUS_CONF OUT_DST_DBUS_CONF)
        # Checks the variable in the template
        if(NOT EXISTS ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in )
            FILE(WRITE ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in 
                       "################################################ CMAKE GENERATED #################################################")
        endif( )
        
        FILE(READ ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in DBUS_CONF_IN)
        if( NOT DBUS_CONF_IN MATCHES ${IN_PLACEHOLDER} )
            if( NOT DBUS_CONF_IN STREQUAL "" )
                FILE(APPEND ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in "\r\n\r\n")
            endif( )   
            FILE(APPEND ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/fidls/AudioManager_dbus.conf.in ${IN_PLACEHOLDER})
        endif( )
        # Reads out the common-api dbus configuration for the node state manager.
        FILE(READ ${IN_SRC_DBUS_CONF} ${OUT_DST_DBUS_CONF})
    ENDMACRO(INSERT_DBUS_CONF_IF_NEEDED) 
    
 ELSEIF(WITH_ENABLED_IPC STREQUAL "NONE")   
    SET (WITH_CAPI_WRAPPER OFF CACHE INTERNAL "hide this!" FORCE)
    SET (WITH_DBUS_WRAPPER OFF CACHE INTERNAL "hide this!" FORCE)
ENDIF ()
    
IF (NOT WITH_DBUS_WRAPPER AND NOT WITH_CAPI_WRAPPER)
    SET (WITH_NSM OFF)
ENDIF (NOT WITH_DBUS_WRAPPER AND NOT WITH_CAPI_WRAPPER)
 
#Can be changed via passing -DDBUS_SERVICE_PREFIX="XXX" to cmake
IF(NOT DEFINED DBUS_SERVICE_PREFIX)
	SET( DBUS_SERVICE_PREFIX "org.genivi.audiomanager\0")
ENDIF(NOT DEFINED DBUS_SERVICE_PREFIX)

#Can be changed via passing -DDBUS_SERVICE_OBJECT_PATH="XXX" to cmake
IF(NOT DEFINED DBUS_SERVICE_OBJECT_PATH)
	SET( DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager\0" )
ENDIF(NOT DEFINED DBUS_SERVICE_OBJECT_PATH)

#Can be changed via passing -DDEFAULT_TELNETPORT="XXX" to cmake
IF(NOT DEFINED DEFAULT_TELNETPORT)
	SET( DEFAULT_TELNETPORT 6080 )
ENDIF(NOT DEFINED DEFAULT_TELNETPORT)

#Can be changed via passing -DMAX_TELNETCONNECTIONS="XXX" to cmake
IF(NOT DEFINED MAX_TELNETCONNECTIONS)
	SET( MAX_TELNETCONNECTIONS 3 )
ENDIF(NOT DEFINED MAX_TELNETCONNECTIONS)

#Can be changed via passing -DNSM_BUS_INTERFACE="XXX" to cmake
IF(NOT DEFINED NSM_BUS_INTERFACE)
	SET( NSM_BUS_INTERFACE "org.genivi.NodeStateManager")
ENDIF(NOT DEFINED NSM_BUS_INTERFACE)

#Can be changed via passing -DNSM_PATH="XXX" to cmake
IF(NOT DEFINED NSM_PATH)
	SET( NSM_PATH "/org/genivi/NodeStateManager")
ENDIF(NOT DEFINED NSM_PATH)

#Can be changed via passing -DNSM_INTERFACE="XXX" to cmake
IF(NOT DEFINED NSM_INTERFACE)
	SET( NSM_INTERFACE "org.genivi.NodeStateManager.Consumer")
ENDIF(NOT DEFINED NSM_INTERFACE)

SET(PLUGINS_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/plugins)
SET(LIB_INSTALL_SUFFIX "audioManager")

if(USE_BUILD_LIBS)
	IF(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
		SET(DEFAULT_PLUGIN_COMMAND_DIR "${PLUGINS_OUTPUT_PATH}/command")
	ENDIF(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)	
	IF(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)	
		SET(DEFAULT_PLUGIN_ROUTING_DIR "${PLUGINS_OUTPUT_PATH}/routing")
	ENDIF(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)		
	IF(NOT DEFINED CONTROLLER_PLUGIN)
		SET(CONTROLLER_PLUGIN "${PLUGINS_OUTPUT_PATH}/control/libPluginControlInterface.so")
	ENDIF(NOT DEFINED CONTROLLER_PLUGIN)
else(USE_BUILD_LIBS)
	IF(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
		SET(DEFAULT_PLUGIN_COMMAND_DIR "${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX}/command")
	ENDIF(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
	IF(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)
		SET(DEFAULT_PLUGIN_ROUTING_DIR "${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX}/routing")
	ENDIF(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)
	IF(NOT DEFINED CONTROLLER_PLUGIN)
		SET(CONTROLLER_PLUGIN "${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX}/control/libPluginControlInterface.so")
	ENDIF(NOT DEFINED CONTROLLER_PLUGIN)
endif(USE_BUILD_LIBS)

IF(EXISTS "ProjectSpecific/")
    SET(PROJECT_INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/ProjectSpecific/include)
endif(EXISTS "ProjectSpecific/")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
SET(AUDIO_INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/include)
SET(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/doc)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/test)
SET(DOXY_PROTOTYPE ${CMAKE_SOURCE_DIR}/cmake/DoxyFile.in)
SET(DOXY_FILE ${CMAKE_CURRENT_BINARY_DIR}/DoxyFile)

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/AudioManagerDaemon/docx/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/DoxyFile )

IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
ENDIF(WITH_DLT)

IF(WITH_TESTS)	
	add_subdirectory(googleMock)
	set(GMOCK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/googleMock/include")
	set(GOOGLE_TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/googleMock/gtest/include")
ENDIF(WITH_TESTS)

IF(WITH_DOCUMENTATION)
	find_package(Doxygen)
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (AudioManangerDoku ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  		SOURCES ${CMAKE_SOURCE_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
	)
ENDIF(WITH_DOCUMENTATION)

##global build flags set(CPACK_RPM_COMPONENT_INSTALL ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=gnu++0x -D_GNU_SOURCE -pedantic -Wno-variadic-macros -Wno-long-long")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=c++98 -D_GNU_SOURCE")

if(WITH_PLUGIN_COMMAND)
    if(WITH_DBUS_WRAPPER)
      	    add_subdirectory (PluginCommandInterfaceDbus)
    elseif(WITH_CAPI_WRAPPER)  	    
    	    add_subdirectory (PluginCommandInterfaceCAPI)
    endif()    
endif(WITH_PLUGIN_COMMAND)

if(WITH_PLUGIN_ROUTING)
	add_subdirectory (PluginRoutingInterfaceAsync)
    if(WITH_DBUS_WRAPPER)
	    add_subdirectory (PluginRoutingInterfaceDbus)
	elseif(WITH_CAPI_WRAPPER)  	    
    	add_subdirectory (PluginRoutingInterfaceCAPI)    
	endif()    
endif(WITH_PLUGIN_ROUTING)

if(WITH_PLUGIN_CONTROL)
	add_subdirectory (PluginControlInterface)
endif(WITH_PLUGIN_CONTROL)

add_subdirectory (AudioManagerDaemon)

IF(EXISTS "${CMAKE_SOURCE_DIR}/ProjectSpecific/")
    add_subdirectory (ProjectSpecific)
endif(EXISTS "${CMAKE_SOURCE_DIR}/ProjectSpecific/")

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Here starts package creation
SET(CPACK_SET_DESTDIR ON)
SET(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_SOURCE_DIR}/packages)
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "AudioManager")
SET(CPACK_PACKAGE_VENDOR "GENIVI")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AudioManager: This component manages audio in the GENIVI context")
SET(CPACK_PACKAGE_VERSION "${DAEMONVERSION}")
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENCE)
SET(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README)
SET(CPACK_PACKAGE_CONTACT "Christian Linke(BMW) christian.linke@bmw.de")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "genivi")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "*")
SET(CPACK_STRIP_FILES TRUE)
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/README)
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_SOURCE_IGNORE_FILES ".settings*" ".cproject" "/\\\\.metadata" "\\\\.#" "/#" ".*~" "/\\\\.git" "${CMAKE_CURRENT_BINARY_DIR}" "bin/" "packages/" "config.h") 

IF(WITH_TESTS)
	get_property(ADD_DEPEND GLOBAL PROPERTY tests_prop)
	list(REMOVE_DUPLICATES ADD_DEPEND)
	list(APPEND ALL_DEPEND ${ADD_DEPEND})
	FOREACH (dep ${ADD_DEPEND})	
		SET(tests_DEPENDENCIES "${dep} ,${tests_DEPENDENCIES}")
	ENDFOREACH(dep)
	STRING(REGEX REPLACE ".$" "" tests_DEPENDENCIES ${tests_DEPENDENCIES})
ENDIF(WITH_TESTS)

#evaluate the properties
if(WITH_MAIN)
get_property(ADD_DEPEND GLOBAL PROPERTY bin_prop)
list(REMOVE_DUPLICATES ADD_DEPEND)
set(ALL_DEPEND ${ADD_DEPEND})
FOREACH (dep ${ADD_DEPEND})
	SET(bin_DEPENDENCIES "${dep} ,${bin_DEPENDENCIES}")
ENDFOREACH(dep)
STRING(REGEX REPLACE ".$" "" bin_DEPENDENCIES ${bin_DEPENDENCIES})
endif(WITH_MAIN)

if(WITH_PLUGIN_COMMAND OR WITH_PLUGIN_CONTROL OR WITH_PLUGIN_ROUTING)
get_property(ADD_DEPEND GLOBAL PROPERTY sampleplugins_prop)
list(REMOVE_DUPLICATES ADD_DEPEND)
list(APPEND ALL_DEPEND ${ADD_DEPEND})
FOREACH (dep ${ADD_DEPEND})
	SET(sampleplugins_DEPENDENCIES "${dep} ,${sampleplugins_DEPENDENCIES}")
ENDFOREACH(dep)
STRING(REGEX REPLACE ".$" "" sampleplugins_DEPENDENCIES ${sampleplugins_DEPENDENCIES})
endif(WITH_PLUGIN_COMMAND OR WITH_PLUGIN_CONTROL OR WITH_PLUGIN_ROUTING)

get_property(ADD_DEPEND GLOBAL PROPERTY dev_prop)
list(REMOVE_DUPLICATES ADD_DEPEND)
list(APPEND ALL_DEPEND ${ADD_DEPEND})
FOREACH (dep ${ADD_DEPEND})
	SET(dev_DEPENDENCIES "${dep} ,${dev_DEPENDENCIES}")
ENDFOREACH(dep)
STRING(REGEX REPLACE ".$" "" dev_DEPENDENCIES ${dev_DEPENDENCIES})

list(REMOVE_DUPLICATES ALL_DEPEND)
list(REMOVE_ITEM ALL_DEPEND "audiomanager-bin")
FOREACH (dep ${ALL_DEPEND})
	SET(all_DEPENDENCIES "${dep} ,${all_DEPENDENCIES}")
ENDFOREACH(dep)
STRING(REGEX REPLACE ".$" "" all_DEPENDENCIES ${all_DEPENDENCIES})
execute_process(COMMAND cp ${CMAKE_MODULE_PATH}/add_package_dependencies.sh ${CMAKE_CURRENT_BINARY_DIR})

#component based dep package generation is only supported above 2.8.5
IF (${CMAKE_VERSION} VERSION_GREATER 2.8.5)

	SET(CPACK_COMPONENTS_ALL bin sampleplugins tests dev)
	SET(CPACK_COMPONENTS_IGNORE_GROUPS 1)
	SET(CPACK_DEB_COMPONENT_INSTALL ON)
	ADD_CUSTOM_TARGET(genivi_package 
		COMMAND ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR} -DUSE_BUILD_LIBS=OFF
		COMMAND make package
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/add_package_dependencies.sh ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux-bin.deb ${CPACK_OUTPUT_FILE_PREFIX} \"${bin_DEPENDENCIES}\"
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/add_package_dependencies.sh ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux-sampleplugins.deb ${CPACK_OUTPUT_FILE_PREFIX} \"${sampleplugins_DEPENDENCIES}\"
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/add_package_dependencies.sh ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux-tests.deb ${CPACK_OUTPUT_FILE_PREFIX} \"${tests_DEPENDENCIES}\"
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/add_package_dependencies.sh ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux-dev.deb ${CPACK_OUTPUT_FILE_PREFIX} \"${dev_DEPENDENCIES}\"
	) 
ELSE (${CMAKE_VERSION} VERSION_GREATER 2.8.5)
	IF(${CMAKE_VERSION} VERSION_GREATER 2.8.3)
		ADD_CUSTOM_TARGET(genivi_package 
			COMMAND ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR} -DUSE_BUILD_LIBS=OFF
			COMMAND make package
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}A logical block opening on t/add_package_dependencies.sh ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux.deb ${CPACK_OUTPUT_FILE_PREFIX} \"${all_DEPENDENCIES}\"
		)
	ELSE(${CMAKE_VERSION} VERSION_GREATER 2.8.3)
		ADD_CUSTOM_TARGET(genivi_package 
			COMMAND ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR} -DUSE_BUILD_LIBS=OFF
			COMMAND make package
			COMMAND mkdir -p ../${CPACK_OUTPUT_FILE_PREFIX}
			COMMAND mv ${CMAKE_CURRENT_BINARY_DIR}/${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux.deb ${CPACK_OUTPUT_FILE_PREFIX}
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/add_package_dependencies.sh ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux.deb ${CPACK_OUTPUT_FILE_PREFIX} \"${all_DEPENDENCIES}\"
		)
	ENDIF(${CMAKE_VERSION} VERSION_GREATER 2.8.3)
ENDIF (${CMAKE_VERSION} VERSION_GREATER 2.8.5)

INCLUDE(CPack)


