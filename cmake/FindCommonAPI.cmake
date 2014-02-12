#
# Copyright (C) 2012, BMW AG
# 
# \author Christian Linke
# 

include(UsePkgConfig)

pkg_check_modules(COMMON_API CommonAPI)
IF(COMMON_API_FOUND)	
    FIND_PATH(COMMON_API_INCLUDE_DIR
              NAMES CommonAPI/Runtime.h CommonAPI/Proxy.h 
              PATH_SUFFIXES CommonAPI-${COMMON_API_VERSION}	 
              PATHS
              ${COMMON_API_INCLUDE_DIRS}	
              "/usr/local/include"       	
              "/usr/include")

    FIND_LIBRARY(COMMON_API_LIBRARY 
                 NAMES CommonAPI
                 PATHS
                 "/usr/local/lib"
                 "/usr/lib"
                )      
ENDIF(COMMON_API_FOUND)

pkg_check_modules(COMMON_API_DBUS CommonAPI-DBus)
IF(COMMON_API_DBUS_FOUND)	
    FIND_PATH(COMMON_API_DBUS_INCLUDE_DIR
              NAMES CommonAPI/DBus/DBusRuntime.h CommonAPI/DBus/DBusProxy.h 
              PATH_SUFFIXES CommonAPI-${COMMON_API_VERSION}	 
              PATHS
              ${COMMON_API_DBUS_INCLUDE_DIRS}	
              "/usr/local/include"       	
              "/usr/include")

    FIND_LIBRARY(COMMON_API_DBUS_LIBRARY 
                 NAMES CommonAPI-DBus murmurhash-internal
                 PATHS
                 "/usr/local/lib"
                 "/usr/lib"
                 )      
ENDIF(COMMON_API_DBUS_FOUND)


SET(COMMON_API_LIBRARIES ${COMMON_API_LIBRARY})

IF(COMMON_API_INCLUDE_DIR AND COMMON_API_LIBRARY)   
   message(STATUS "Found CommonAPI ${COMMON_API_VERSION}")
ELSE(COMMON_API_LIBRARIES AND COMMON_API_LIBRARY)   
   message(STATUS " CommonAPI not found.")
ENDIF(COMMON_API_INCLUDE_DIR AND COMMON_API_LIBRARY)

SET(COMMON_API_DBUS_LIBRARIES ${COMMON_API_DBUS_LIBRARY})

IF(COMMON_API_DBUS_INCLUDE_DIR AND COMMON_API_DBUS_LIBRARY)   
   message(STATUS "Found CommonAPI_DBUS")
ELSE(COMMON_API_DBUS_INCLUDE_DIR AND COMMON_API_DBUS_LIBRARY)   
   message(STATUS " CommonAPI_DBUS not found.")
ENDIF(COMMON_API_DBUS_INCLUDE_DIR AND COMMON_API_DBUS_LIBRARY)
				
MARK_AS_ADVANCED(
    COMMON_API_DBUS_LIBRARIES 
    COMMON_API_DBUS_INCLUDE_DIR
)		
		
MARK_AS_ADVANCED(
    COMMON_API_LIBRARIES 
    COMMON_API_INCLUDE_DIR
    COMMON_API_GEN_INCLUDE_DIR
    COMMON_API_GEN_SOURCES
)
