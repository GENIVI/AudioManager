cmake_minimum_required(VERSION 2.6)

PROJECT(PlayerGui)

set(CMAKE_CXX_FLAGS "-g -pipe -g -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../dbusInterfaces)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/PlayerGUI)

cmake_policy(SET CMP0015 NEW)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
pkg_check_modules(GSTREAMERPLUGINS REQUIRED gstreamer-plugins-base-0.10)
pkg_check_modules(PHONON REQUIRED phonon)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${STD_INCLUDE_DIRS})

# add Qt modules here, the include will setup QT_LIBRARIES
SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTPHONON TRUE)

INCLUDE(${QT_USE_FILE})

SET( UI_FILES
     	playergui.ui
)

#all source files go here
SET(PLAYERGUI_SRCS_CXX
    	main.cpp 
    	playerGui.cpp
    	player.cpp
    	${DBUS_FOLDER}/DBusTypes.cpp 
    	${CMAKE_CURRENT_BINARY_DIR}/DBusSend.cpp

)

INCLUDE_DIRECTORIES(
	include
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${GSTREAMER_INLCUDE_DIRS}
	${GSTREAMERDEV_INLCUDE_DIRS}
	${GSTREAMERPLUGINS_INCLUDE_DIRS}
	${PHONON_INCLUDE_DIRS}
	${DBUS_FOLDER}
)

#every header that contains QT stuff like Q_OBJECT .... and must be mocced
SET(PLAYERGUI_MOC_SRCS
	${DBUS_FOLDER}/DBusTypes.h 
	playerGui.h
	player.h
    	${CMAKE_CURRENT_BINARY_DIR}/DBusSend.h

)

LINK_DIRECTORIES(
	${LINK_DIRECTORIES}
	${GLIB_LIBRARY_DIRS}
	${GSTREAMER_LIBRARY_DIRS}
	${GSTREAMER_PLUGINS_LIBRARY_DIRS}
	${PHONON_LIBRARY_DIRS}
)

QT4_WRAP_UI(UI_HDRS ${UI_FILES} )


#now take the headers, moc them and append the resulting moc files to the sources
QT4_WRAP_CPP(PLAYERGUI_SRCS_CXX ${PLAYERGUI_MOC_SRCS})

ADD_EXECUTABLE(PlayerGui ${PLAYERGUI_SRCS_CXX} ${UI_HDRS})

TARGET_LINK_LIBRARIES(PlayerGui 
	${QT_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${PHONON_LIBRARIES}
)

add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusSend_.h ${CMAKE_CURRENT_BINARY_DIR}/DBusSend.cpp
	COMMAND qdbusxml2cpp ARGS -v -c DBusSend -p DBusSend.h:DBusSend.cpp ${DBUS_FOLDER}/DBusAudioManager.xml
	MAIN_DEPENDENCY ${DBUS_FOLDER}/DBusAudioManager.xml
)

add_custom_command(
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusSend.h
	COMMAND sed ARGS -i  '1i \#include \"../dbusInterfaces/DBusTypes.h\"' DBusSend.h
	MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/DBusSend_.h
)

#add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  	configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Docs ALL 
  	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  	SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(DOXYGEN_FOUND)


