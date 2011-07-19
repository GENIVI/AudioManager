cmake_minimum_required(VERSION 2.6)

PROJECT(PluginHookStandard)

set(CMAKE_CXX_FLAGS "-g -pipe -fPIC -Wall -W -D_REENTRANT")

set(STD_INCLUDE_DIRS "/usr/include")
set(EXECUTABLE_OUTPUT_PATH ../../bin/)
set(LIBRARY_OUTPUT_PATH ../plugins)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../doc/PluginHookStandard)
set(DOCUMENT $ENV{gendoc})

cmake_policy(SET CMP0015 NEW)

file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR} 
	${STD_INCLUDE_DIRS}
 	${CMAKE_CURRENT_BINARY_DIR}
	../AudioManagerDaemon
)

# all source files go here
SET(PLUGIN_STANDARD_HOOKS_SRCS_CXX
    StandardHook.cpp
)

add_library(PluginHookStandard SHARED ${PLUGIN_STANDARD_HOOKS_SRCS_CXX})
add_library(PluginHookStandardStatic ${PLUGIN_STANDARD_HOOKS_SRCS_CXX})

#TARGET_LINK_LIBRARIES(PluginHookStandard)

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

