cmake_minimum_required(VERSION 2.6)

PROJECT(PluginStructuralTest)

set(CMAKE_CXX_FLAGS "-g -pipe -fPIC -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins)
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../dbusInterfaces)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/PluginTest)

cmake_policy(SET CMP0015 NEW)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(Qt4 REQUIRED)

# add Qt modules here, the include will setup QT_LIBRARIES
SET(QT_USE_QTSQL TRUE)

INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
 	${CMAKE_CURRENT_BINARY_DIR}
 	${QT_INCLUDE_DIR} 
 	${DBUS_FOLDER}
	../AudioManagerDaemon
)

# defines for QL Plugins
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_STATICPLUGIN)
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

INCLUDE(${QT_USE_FILE})

# all source files go here
SET(Test_SRCS_CXX
    Test.cpp
)

# every header that contains QT stuff like Q_OBJECT .... and must be mocced
SET(Test_MOC_SRCS
    Test.h
)

#now take the headers, moc them and append the resulting moc files to the sources
QT4_WRAP_CPP(Test_SRCS_CXX ${Test_MOC_SRCS})

add_library(TestPlugin ${Test_SRCS_CXX})

TARGET_LINK_LIBRARIES(TestPlugin 
	${QT_LIBRARIES}
)

#add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  	configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Docs ALL 
  	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  	SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(DOXYGEN_FOUND)

