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
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(DOC_OUTPUT_PATH ${DOC_OUTPUT_PATH}/AudioManager)
set(INCLUDES_FOLDER "include")

pkg_check_modules(SQLITE REQUIRED sqlite3)

IF(WITH_DBUS_WRAPPER)
    FIND_PACKAGE(DBUS REQUIRED)
ENDIF(WITH_DBUS_WRAPPER)

SET(AUDIOMAN_SRCS_CXX
    src/CommandReceiver.cpp
    src/CommandSender.cpp
    src/ControlReceiver.cpp
    src/ControlSender.cpp
    src/DatabaseHandler.cpp
    src/DatabaseObserver.cpp
    src/main.cpp
    src/RoutingReceiver.cpp
    src/RoutingSender.cpp
    src/SocketHandler.cpp
)

IF(WITH_DBUS_WRAPPER)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/DBusWrapper.cpp
    )
ENDIF(WITH_DBUS_WRAPPER)

IF(WITH_SOCKETHANDLER_LOOP)
    SET (AUDIOMAN_SRCS_CXX
        ${AUDIOMAN_SRCS_CXX}
        src/SocketHandler.cpp
     )
ENDIF(WITH_SOCKETHANDLER_LOOP)

INCLUDE_DIRECTORIES(	
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
	${DLT_INCLUDE_DIRS}
	${AUDIO_INCLUDES_FOLDER}
	${INCLUDES_FOLDER}
)

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
	${DLT_LIBRARIES}
	${DBUS_LIBRARY}
)
ELSE(WITH_DBUS_WRAPPER)
    TARGET_LINK_LIBRARIES(AudioManager 
        ${SQLITE_LIBRARIES}
        ${DLT_LIBRARIES}
)
ENDIF(WITH_DBUS_WRAPPER)

IF(WITH_TESTS)
	add_subdirectory (test)
ENDIF(WITH_TESTS)

IF(WITH_DOCUMENTATION)
	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
  	add_custom_target (Docs ALL 
  		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
  		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
	)
ENDIF(WITH_DOCUMENTATION)




