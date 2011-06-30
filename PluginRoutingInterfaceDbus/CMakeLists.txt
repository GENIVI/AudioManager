cmake_minimum_required(VERSION 2.6)

PROJECT(AudioManagerDeamon)

set(CMAKE_CXX_FLAGS "-g -pipe -fPIC -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins)
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../dbusInterfaces)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/DBusPlugin)

cmake_policy(SET CMP0015 NEW)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
 	${CMAKE_CURRENT_BINARY_DIR}
	${DBUS_FOLDER}
	../AudioManagerDeamon
)

# add Qt modules here, the include will setup QT_LIBRARIES

# defines for QL Plugins
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_STATICPLUGIN)
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)


INCLUDE(${QT_USE_FILE})

# all source files go here
SET(PLUGINDBUS_SRCS_CXX
    DBusInterface.cpp 
    RoutingSend.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/DBusSend.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/DBusInterfaceAdaptor.cpp 
)

# every header that contains QT stuff like Q_OBJECT .... and must be mocced
SET(PLUGINDBUS_MOC_SRCS
    DBusInterface.h 
    RoutingSend.h 
    ${CMAKE_CURRENT_BINARY_DIR}/DBusSend.h
    ${CMAKE_CURRENT_BINARY_DIR}/DBusInterfaceAdaptor.h 
)

#now take the headers, moc them and append the resulting moc files to the sources
QT4_WRAP_CPP(PLUGINDBUS_SRCS_CXX ${PLUGINDBUS_MOC_SRCS})

add_library(RoutingPlugin ${PLUGINDBUS_SRCS_CXX})

TARGET_LINK_LIBRARIES(RoutingPlugin
	${QT_LIBRARIES}
)

add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusInterfaceAdaptor.h ${CMAKE_CURRENT_BINARY_DIR}/DBusInterfaceAdaptor.cpp
	COMMAND qdbusxml2cpp ARGS -c DBusInterfaceAdaptor -a DBusInterfaceAdaptor.h:DBusInterfaceAdaptor.cpp ${DBUS_FOLDER}/DBusReceive.xml 
	MAIN_DEPENDENCY ${DBUS_FOLDER}/DBusReceive.xml
)

add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusSend.h ${CMAKE_CURRENT_BINARY_DIR}/DBusSend.cpp
	COMMAND qdbusxml2cpp ARGS -v -c DBusSend -p DBusSend.h:DBusSend.cpp ${DBUS_FOLDER}/DBusSend.xml
	MAIN_DEPENDENCY ${DBUS_FOLDER}/DBusSend.xml
)

#add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  	configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Docs ALL 
  	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  	SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(DOXYGEN_FOUND)



