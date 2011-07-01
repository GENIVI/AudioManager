cmake_minimum_required(VERSION 2.6)

PROJECT(PluginRoutingInterfaceTCP)

set(CMAKE_CXX_FLAGS "-g -pipe -fPIC -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins)
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../dbusInterfaces)

cmake_policy(SET CMP0015 NEW)

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)

INCLUDE_DIRECTORIES(
	include
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${DBUS_FOLDER}
	../AudioManagerDaemon
)

LINK_DIRECTORIES(
	${LINK_DIRECTORIES}
	${GLIB_LIBRARY_DIRS}
)

# add Qt modules here, the include will setup QT_LIBRARIES
SET(QT_USE_QTNETWORK TRUE)

# defines for QL Plugins
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_STATICPLUGIN)
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)



INCLUDE(${QT_USE_FILE})

# all source files go here
SET(PLUGINJACK_SRCS_CXX 
    RoutingSend.cpp 
    tcpClient.cpp
    TcpServer.cpp
)

# every header that contains QT stuff like Q_OBJECT .... and must be mocced
SET(PLUGINJACK_MOC_SRCS
    RoutingSend.h 
    tcpClient.h
    TcpServer.h
)

#now take the headers, moc them and append the resulting moc files to the sources
QT4_WRAP_CPP(PLUGINJACK_SRCS_CXX ${PLUGINJACK_MOC_SRCS})

add_library(RoutingTcpPlugin ${PLUGINJACK_SRCS_CXX})

TARGET_LINK_LIBRARIES(RoutingTcpPlugin
	${QT_LIBRARIES}
)




