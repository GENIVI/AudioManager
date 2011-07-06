cmake_minimum_required(VERSION 2.6)

PROJECT(PluginRoutingInterfaceJack)

set(CMAKE_CXX_FLAGS "-g -pipe -fPIC -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/JackPlugin)
set(DOCUMENT $ENV{gendoc})

cmake_policy(SET CMP0015 NEW)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(JACK REQUIRED jack)

INCLUDE_DIRECTORIES(
	include
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${JACK_INCLUDE_DIRS}
	../AudioManagerDaemon
)

LINK_DIRECTORIES(
	${LINK_DIRECTORIES}
	${GLIB_LIBRARY_DIRS}
	${JACK_LIBRARY_DIRS}
)

# add Qt modules here, the include will setup QT_LIBRARIES

# defines for QL Plugins
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_STATICPLUGIN)
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)



INCLUDE(${QT_USE_FILE})

# all source files go here
SET(PLUGINJACK_SRCS_CXX 
    RoutingSend.cpp 
    JackAudioController.cpp
)

# every header that contains QT stuff like Q_OBJECT .... and must be mocced
SET(PLUGINJACK_MOC_SRCS
    RoutingSend.h 
)

#now take the headers, moc them and append the resulting moc files to the sources
QT4_WRAP_CPP(PLUGINJACK_SRCS_CXX ${PLUGINJACK_MOC_SRCS})

add_library(RoutingJackPlugin ${PLUGINJACK_SRCS_CXX})

TARGET_LINK_LIBRARIES(RoutingJackPlugin
	${QT_LIBRARIES}
	${JACK_LIBRARIES}
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



