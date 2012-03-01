#GENIVI AUDIOMANAGER
#
#Copyright (C) 2011, BMW AG
#
#Datum  20.2.2012
#author Christian Mueller (christian.ei.mueller@bmw.de)
#
#***********************************************************************************************************
#LICENSE
#***********************************************************************************************************
#
#GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
#Copyright (C) 2011, BMW AG – Christian Mueller  Christian.ei.mueller@bmw.de
#
#This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
#This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
#You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
#Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
#Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
#As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
#Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
#

cmake_minimum_required(VERSION 2.6)

PROJECT(AudioManagerDeamon)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/AudioManager)
set(include_FOLDER "include")

pkg_check_modules(SQLITE REQUIRED sqlite3)

IF(WITH_DBUS_WRAPPER)
    FIND_PACKAGE(DBUS REQUIRED)
ENDIF(WITH_DBUS_WRAPPER)

SET(AUDIOMAN_SRCS_CXX
    src/main.cpp
    src/CAmCommandReceiver.cpp
    src/CAmCommandSender.cpp
    src/CAmControlReceiver.cpp
    src/CAmControlSender.cpp
    src/CAmDatabaseHandler.cpp
    src/CAmDatabaseObserver.cpp
    src/CAmRoutingReceiver.cpp
    src/CAmRoutingSender.cpp
    src/CAmSocketHandler.cpp
    src/CAmTelnetServer.cpp
    src/CAmRouter.cpp
    src/CAmDltWrapper.cpp
    src/CAmTelnetMenuHelper.cpp
)

IF(WITH_DBUS_WRAPPER)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmDbusWrapper.cpp
    )
ENDIF(WITH_DBUS_WRAPPER)

IF(WITH_SOCKETHANDLER_LOOP)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/CAmSocketHandler.cpp
     )
ENDIF(WITH_SOCKETHANDLER_LOOP)

INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}	
	${AUDIO_include_FOLDER}
	${include_FOLDER}
)

IF(WITH_DLT)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${DLT_INCLUDE_DIRS}
)
ENDIF(WITH_DLT)

IF(WITH_DBUS_WRAPPER)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${DBUS_ARCH_INCLUDE_DIR}
    ${DBUS_INCLUDE_DIR}
)
ENDIF(WITH_DBUS_WRAPPER)

ADD_EXECUTABLE(AudioManager ${AUDIOMAN_SRCS_CXX})

IF(WITH_DBUS_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
    	${SQLITE_LIBRARIES}
    	${DBUS_LIBRARY}
    )
ELSE(WITH_DBUS_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
        ${SQLITE_LIBRARIES}
    )
ENDIF(WITH_DBUS_WRAPPER)

IF(WITH_DLT)
    TARGET_LINK_LIBRARIES(AudioManager
        ${DLT_LIBRARIES}
    )
ENDIF(WITH_DLT)  

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

#IF(WITH_DOCUMENTATION)
#	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
#	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
#  	add_custom_target (Docs ALL 
#  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
#  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
#	)
#ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS AudioManager 
        RUNTIME 
        DESTINATION bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT bin
)

INSTALL(DIRECTORY "${AUDIO_include_FOLDER}/" 
        DESTINATION include
        COMPONENT dev
)

# General packet dependency
SET(ADD_DEPEND_BIN_PROP "sqlite3(>=3.6.22)")

# Optional packet dependency
IF(WITH_DLT)
    SET(ADD_DEPEND_BIN_PROP ${ADD_DEPEND_BIN_PROP} "dlt")
ENDIF(WITH_DLT)

IF(WITH_DBUS_WRAPPER)
    SET(ADD_DEPEND_BIN_PROP ${ADD_DEPEND_BIN_PROP} "libdbus-1-3(>=1.2.16)")
ENDIF()

# Additional Dev packet dependency
SET(ADD_DEPEND_DEV_PROP "audiomanager-bin" ${ADD_DEPEND_BIN_PROP})

MESSAGE(STATUS "bin packet dependency: ${ADD_DEPEND_BIN_PROP}")
MESSAGE(STATUS "dev dependency: ${ADD_DEPEND_DEV_PROP}")

set_property(GLOBAL APPEND PROPERTY bin_prop "${ADD_DEPEND_BIN_PROP}")
set_property(GLOBAL APPEND PROPERTY dev_prop "${ADD_DEPEND_DEV_PROP}")



