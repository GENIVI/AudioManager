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

set(CMAKE_CXX_FLAGS "-g -pipe -g -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(DOC_OUTPUT_PATH ../../doc/)
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../dbusInterfaces)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/AudioManager)
set(DOCUMENT $ENV{gendoc})

cmake_policy(SET CMP0015 NEW)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(PkgConfig)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(DLT REQUIRED automotive-dlt)
pkg_check_modules(JACK REQUIRED jack)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${STD_INCLUDE_DIRS})

#all source files go here
SET(AUDIOMAN_SRCS_CXX
	AudioManagerCore.cpp
    RoutingReceive.cpp
    Router.cpp 
    DataBaseHandler.cpp
    main.cpp
    DBusCommandInterface.cpp
    HookEngine.cpp
    PluginHandler.cpp
)

INCLUDE_DIRECTORIES(
	include
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
	${JACK_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${DBUS_FOLDER}
	${DLT_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${LINK_DIRECTORIES}
	${PULSE_LIBRARY_DIRS}
	${JACK_LIBRARY_DIRS}
	${DBUS_FOLDER}
	${PROJECT_BINARY_DIR}/../plugins
)

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX})

TARGET_LINK_LIBRARIES(AudioManager 
	${SQLITE_LIBRARIES}
	${DLT_LIBRARIES}
)

add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusCommand_.h ${CMAKE_CURRENT_BINARY_DIR}/DBusCommand.cpp
	COMMAND qdbusxml2cpp ARGS -c DBusCommand -a DBusCommand.h:DBusCommand.cpp ${DBUS_FOLDER}/DBusAudioManager.xml
	MAIN_DEPENDENCY ${DBUS_FOLDER}/DBusAudioManager.xml
)


add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusCommand.h
	COMMAND sed ARGS -i  '1i \#include \"../../dbusInterfaces/DBusTypes.h\"' DBusCommand.h
	MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/DBusCommand_.h
)

#add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
if(DOCUMENT)
  	configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Docs ALL 
  	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  	SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(DOCUMENT)
endif(DOXYGEN_FOUND)




