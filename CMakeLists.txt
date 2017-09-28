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


cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include ( MacroVersionFromGit )
include ( MacroInterfaceVersions )

project(AudioManager LANGUAGES C CXX VERSION ${DAEMONVERSION})

include ( CMakeDependentOption )
include ( CMakePackageConfigHelpers )
include ( GNUInstallDirs )
include ( CTest )
find_package(PkgConfig)

option( WITH_TESTS
	"Build together with all available unitTest" ON )

option( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

option( WITH_DLT
    "Enable automotive-DLT Support" ON )

option( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

option ( WITH_SYSTEMD_WATCHDOG
    "Build with systemD support & watchdog" ON)

option ( GLIB_DBUS_TYPES_TOLERANT
    "Build dbus with tolerance towards glib 16bit/32bit handling" ON)

option ( WITH_CAPI_WRAPPER
	"Build with commonapi wrapper" ON)

option ( WITH_DBUS_WRAPPER
	"Build with Dbus wrapper" OFF)

option ( WITH_SHARED_UTILITIES
    "Build audio manager utilities as dynamic library" OFF)

option ( WITH_SHARED_CORE
    "Build audio manager core as dynamic library" OFF)
    
option ( WITH_TIMERFD
    "Build with the linux specific TIMERFD feature to support timing without using signals" ON)
    

set(DBUS_SERVICE_PREFIX "org.genivi.audiomanager"
    CACHE PROPERTY "The dbus service prefix for the AM - only changable for legacy dbus")

set(DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager"
    CACHE PROPERTY "The service object path of the AM - only changable for legacy dbus")

set(DYNAMIC_ID_BOUNDARY 101
    CACHE INTEGER "The ID boundary defines the starting point of the dynamic ID range")

set(LIB_INSTALL_SUFFIX "audiomanager"
    CACHE STRINGS "The suffix used for installation of the plugins")

set(TEST_EXECUTABLE_INSTALL_PATH ${CMAKE_INSTALL_BINDIR}
    CACHE STRINGS "The test binaries will be installed here")

set(DOC_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/doc/AudioManager
    CACHE STRINGS "The doxygen documentation will be placed here")

set(AM_SHARE_FOLDER ${CMAKE_INSTALL_DATAROOTDIR}/audiomanager
    CACHE STRINGS "The share folder for the AM. Some DBus xmls will be placed here and so on")

set(AM_MAP_CAPACITY 10
    CACHE INTEGER "Number of preallocations for datastorage")

set(AM_MAX_CONNECTIONS 0x1000
    CACHE INTEGER "Number of max connections before rollover")

set(AM_MAX_MAIN_CONNECTIONS 0x1000
    CACHE INTEGER "Number of max Mainconnections before rollover")
    
set(AUDIOMANGER_APP_ID "AUDI"
    CACHE PROPERTY "The application ID that is used by the audiomanager")   
    
set(AUDIOMANGER_APP_DESCRIPTION "Audiomanager"
    CACHE PROPERTY "The description of the Audiomanager application")     

set(AUDIOMANAGER_INCLUDE_FOLDER  ${CMAKE_BINARY_DIR}/include)
set(AUDIO_INCLUDE_FOLDER ${CMAKE_SOURCE_DIR}/include)
set(AUDIOMANAGER_UTILITIES ${CMAKE_SOURCE_DIR}/AudioManagerUtilities)
set(AUDIOMANAGER_CORE ${CMAKE_SOURCE_DIR}/AudioManagerCore)
set(AUDIOMANAGER_CORE_INCLUDE ${AUDIOMANAGER_CORE}/include)
set(DOXY_PROTOTYPE ${CMAKE_SOURCE_DIR}/cmake/DoxyFile.in)
set(DOXY_FILE ${CMAKE_CURRENT_BINARY_DIR}/DoxyFile)
set(AUDIOMANAGER_DAEMON_FOLDER ${CMAKE_SOURCE_DIR}/AudioManagerDaemon)
set(GOOGLE_MOCK_PROJECT_FOLDER ${CMAKE_SOURCE_DIR}/googleMock)
set(TCLAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/tclap-1.2.1/include)

GET_COMMAND_INTERFACE_VERSION(${AUDIO_INCLUDE_FOLDER})
GET_ROUTING_INTERFACE_VERSION(${AUDIO_INCLUDE_FOLDER})
GET_CONTROL_INTERFACE_VERSION(${AUDIO_INCLUDE_FOLDER})

if(NOT DEFINED DEFAULT_PLUGIN_DIR)
    set(DEFAULT_PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/${LIB_INSTALL_SUFFIX}")
endif(NOT DEFINED DEFAULT_PLUGIN_DIR)
if(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
    set(DEFAULT_PLUGIN_COMMAND_DIR "${DEFAULT_PLUGIN_DIR}/command")
endif(NOT DEFINED DEFAULT_PLUGIN_COMMAND_DIR)
if(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)
    set(DEFAULT_PLUGIN_ROUTING_DIR "${DEFAULT_PLUGIN_DIR}/routing")
endif(NOT DEFINED DEFAULT_PLUGIN_ROUTING_DIR)
if(NOT DEFINED CONTROLLER_PLUGIN_DIR)
    set(CONTROLLER_PLUGIN_DIR "${DEFAULT_PLUGIN_DIR}/control")
endif(NOT DEFINED CONTROLLER_PLUGIN_DIR)


##global build flags set(CPACK_RPM_COMPONENT_INSTALL ON)
set (AUDIOMANAGER_CMAKE_CXX_FLAGS "-std=c++11 -pedantic -rdynamic -Wno-variadic-macros")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${AUDIOMANAGER_CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")


if(WITH_TESTS)
	#check if we can find the google stuff
	pkg_check_modules (GTEST "gtest >= 1.6.0")
	pkg_check_modules (GMOCK "gmock >= 1.6.0")
	find_package (Threads)
	if (NOT("${GTEST_FOUND}" AND "${GMOCK_FOUND}"))
		message (STATUS "Building and installing with shipped sources")
		add_subdirectory(googleMock)
		set(GMOCK_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/googleMock/include")
		set(GTEST_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/googleMock/gtest/include")
		set(GTEST_LIBRARIES gtest gtest_main)
		set(GMOCK_LIBRARIES gmock gmock_main)
		configure_file( ${CMAKE_SOURCE_DIR}/cmake/gtest.pc.in ${CMAKE_BINARY_DIR}/gtest.pc @ONLY )
		configure_file( ${CMAKE_SOURCE_DIR}/cmake/gmock.pc.in ${CMAKE_BINARY_DIR}/gmock.pc @ONLY )
        install(FILES ${CMAKE_BINARY_DIR}/gmock.pc ${CMAKE_BINARY_DIR}/gtest.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)
	endif (NOT("${GTEST_FOUND}" AND "${GMOCK_FOUND}"))
endif(WITH_TESTS)

configure_package_config_file (
    ${CMAKE_SOURCE_DIR}/cmake/AudioManagerConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/AudioManagerConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUFFIX}/cmake
    PATH_VARS AUDIO_INCLUDE_FOLDER )

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AudioManagerConfigVersion.cmake
    VERSION ${DAEMONVERSION}
    COMPATIBILITY SameMajorVersion )

configure_file( ${CMAKE_SOURCE_DIR}/docx/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/DoxyFile )
configure_file( ${CMAKE_SOURCE_DIR}/cmake/config.cmake.in ${CMAKE_BINARY_DIR}/include/audiomanagerconfig.h )
configure_file( ${CMAKE_SOURCE_DIR}/cmake/audiomanager.pc.in ${CMAKE_BINARY_DIR}/audiomanager.pc @ONLY )

add_subdirectory (AudioManagerUtilities)
add_subdirectory (AudioManagerCore)
add_subdirectory (AudioManagerDaemon)


install(
    FILES ${CMAKE_BINARY_DIR}/AudioManagerConfig.cmake
          ${CMAKE_BINARY_DIR}/AudioManagerConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/AudioManager
)

install(FILES ${CMAKE_BINARY_DIR}/audiomanager.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT devel)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/audiomanagerconfig.h
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${LIB_INSTALL_SUFFIX}
    COMPONENT dev)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/tclap-1.2.1/include/tclap
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${LIB_INSTALL_SUFFIX}
    COMPONENT dev)


if(WITH_DOCUMENTATION)
	find_package(Doxygen)
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
	configure_file("README.html" ${DOC_OUTPUT_PATH}/html/README.html)
  	add_custom_target (AudioManangerDoku ALL
  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  		SOURCES ${CMAKE_SOURCE_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
	)
	find_program(ASCIIDOC asciidoc asciidoc.py)

	if (ASCIIDOC_FOUND)
		execute_process(COMMAND ${ASCIIDOC} -o ${DOC_OUTPUT_PATH}/html/README.html ${CMAKE_SOURCE_DIR}/README )
	endif(ASCIIDOC_FOUND)

	install (DIRECTORY ${DOC_OUTPUT_PATH}/
		DESTINATION "${CMAKE_INSTALL_DOCDIR}"
		PATTERN "Doxyfile" EXCLUDE
		PATTERN "def" EXCLUDE)
endif(WITH_DOCUMENTATION)


message(STATUS)
message(STATUS "${PROJECT_NAME} Configuration:")
message(STATUS "CMAKE_BUILD_TYPE              = ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX          = ${CMAKE_INSTALL_PREFIX}")
message(STATUS "BUILD_DOCUMENTATION           = ${WITH_DOCUMENTATION}")
message(STATUS "WITH_TESTS                    = ${WITH_TESTS}")
message(STATUS "WITH_DLT                      = ${WITH_DLT}")
message(STATUS "WITH_TESTS                    = ${WITH_TESTS}")
message(STATUS "WITH_SYSTEMD_WATCHDOG         = ${WITH_SYSTEMD_WATCHDOG}")
message(STATUS "WITH_CAPI_WRAPPER             = ${WITH_CAPI_WRAPPER}")
message(STATUS "WITH_DBUS_WRAPPER             = ${WITH_DBUS_WRAPPER}")
message(STATUS "WITH_SHARED_UTILITIES         = ${WITH_SHARED_UTILITIES}")
message(STATUS "WITH_SHARED_CORE              = ${WITH_SHARED_CORE}")
message(STATUS "WITH_TIMERFD                  = ${WITH_TIMERFD}")
message(STATUS "DYNAMIC_ID_BOUNDARY           = ${DYNAMIC_ID_BOUNDARY}")
message(STATUS "LIB_INSTALL_SUFFIX            = ${LIB_INSTALL_SUFFIX}")
message(STATUS "TEST_EXECUTABLE_INSTALL_PATH  = ${TEST_EXECUTABLE_INSTALL_PATH}")
message(STATUS "DEFAULT_PLUGIN_COMMAND_DIR    = ${DEFAULT_PLUGIN_COMMAND_DIR}")
message(STATUS "DEFAULT_PLUGIN_ROUTING_DIR    = ${DEFAULT_PLUGIN_ROUTING_DIR}")
message(STATUS "DEFAULT_PLUGIN_DIR            = ${DEFAULT_PLUGIN_DIR}")
message(STATUS "CONTROLLER_PLUGIN_DIR         = ${CONTROLLER_PLUGIN_DIR}")
message(STATUS "AM_SHARE_FOLDER               = ${AM_SHARE_FOLDER}")
message(STATUS "AM_MAP_CAPACITY               = ${AM_MAP_CAPACITY}")
message(STATUS "AM_MAX_CONNECTIONS            = ${AM_MAX_CONNECTIONS}")
message(STATUS "AM_MAX_MAIN_CONNECTIONS       = ${AM_MAX_MAIN_CONNECTIONS}")
message(STATUS "BUILD_TESTING                 = ${BUILD_TESTING}")
message(STATUS "CMAKE_INSTALL_DOCDIR          = ${CMAKE_INSTALL_DOCDIR}")
message(STATUS "AUDIOMANGER_APP_ID            = ${AUDIOMANGER_APP_ID}")
message(STATUS "AUDIOMANGER_APP_DESCRIPTION   = ${AUDIOMANGER_APP_DESCRIPTION}")
message(STATUS "CommandInterface version: ${LIB_COMMAND_INTERFACE_VERSION}")
message(STATUS "ControlInterface version: ${LIB_CONTROL_INTERFACE_VERSION}")
message(STATUS "RoutingInterface version: ${LIB_ROUTING_INTERFACE_VERSION}")
message(STATUS)
message(STATUS)
message(STATUS)
