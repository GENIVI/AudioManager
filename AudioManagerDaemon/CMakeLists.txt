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

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")


##todo only when dbus needed!
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

set(STD_INCLUDE_DIRS "/usr/include")
set(DBUS_FOLDER ${CMAKE_SOURCE_DIR}/../dbusInterfaces)
set(AUDIO_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../includes)
set(DBUS_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../dbusIncludes)
set(INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/include)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/AudioManager)
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(DOC_OUTPUT_PATH ../../doc/)
set(DOCUMENT $ENV{gendoc})

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

FIND_PACKAGE(PkgConfig)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(DLT REQUIRED automotive-dlt)

##todo: only when dbus is reuqired !
find_package (Threads)
FIND_PACKAGE(DBUS REQUIRED)

file(GLOB AUDIOMAN_SRCS_CXX "src/*.cpp")

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

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX})

##todo only when dbus is required
TARGET_LINK_LIBRARIES(AudioManager 
	${SQLITE_LIBRARIES}
	${DLT_LIBRARIES}
	${DBUS_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
)

add_subdirectory (test)

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




