cmake_minimum_required(VERSION 2.6)

PROJECT(PluginCommandInterfaceDbus)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xlinker -export-dynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pipe -g -Wall -W -D_REENTRANT")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins/command)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/CommandDBusPlugin)
set(AUDIO_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../includes)
set(DBUS_INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/../dbusIncludes)
set(INCLUDES_FOLDER ${CMAKE_SOURCE_DIR}/include)

FIND_PACKAGE(DBUS REQUIRED)
FIND_PACKAGE(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)


set(DOCUMENT $ENV{gendoc})

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
 	${CMAKE_CURRENT_BINARY_DIR}
 	${DBUS_INCLUDE_DIR}
	${DBUS_ARCH_INCLUDE_DIR}
	${AUDIO_INCLUDES_FOLDER}
	${INCLUDES_FOLDER}
	${DBUS_INCLUDES_FOLDER}
)

# all source files go here
file(GLOB PLUGINDBUS_SRCS_CXX "src/*.cpp")

add_library(PluginCommandInterfaceDbus SHARED ${PLUGINDBUS_SRCS_CXX})

TARGET_LINK_LIBRARIES(PluginCommandInterfaceDbus 
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
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



