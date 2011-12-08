 #
 #  Copyright (C) 2011, BMW AG 
 # 
 # AudioManagerDeamon Test
 #
 # @file CMakeLists.txt
 #
 # @date: 20.05.2011
 # @author: Christian Müller (christian.ei.mueller@bmw.de)
 #

cmake_minimum_required(VERSION 2.6)

PROJECT(AudioManagerDeamon)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xlinker -export-dynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "-g -pipe -g -Wall -W -D_REENTRANT")

##todo only when dbus needed!
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

set(STD_INCLUDE_DIRS "/usr/include")
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../../dbusInterfaces)
set(AUDIO_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../../includes)
set(DBUS_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../../dbusIncludes)
set(INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../include)
set(EXECUTABLE_OUTPUT_PATH ../../../test/)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(PkgConfig)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(DLT REQUIRED automotive-dlt)

file(GLOB DATABASE_SRCS_CXX "../src/DatabaseHandler.cpp")

INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${DLT_INCLUDE_DIRS}
	${DBUS_INCLUDE_DIR}
	${AUDIO_INCLUDES_FOLDER}
	${INCLUDES_FOLDER}
	${DBUS_ARCH_INCLUDE_DIR}
	${DBUS_INCLUDES_FOLDER}
)

ADD_EXECUTABLE(databasetest databasetest.cpp ${DATABASE_SRCS_CXX})

TARGET_LINK_LIBRARIES(databasetest 
	${SQLITE_LIBRARIES}
	${DLT_LIBRARIES}
	${DBUS_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	gtest
)



