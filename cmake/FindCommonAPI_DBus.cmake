#
# Copyright (C) 2012, BMW AG
# 
# \author Christian Linke
# 

include(UsePkgConfig)

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
                                                               
ELSE(COMMON_API_DBUS_FOUND)
    
    MESSAGE(STATUS "CommonAPI_DBUS package not found, search directly, trying version 0.7 ...")

      
    FIND_PATH(COMMON_API_DBUS_INCLUDE_DIR
              NAMES CommonAPI/DBus/DBusRuntime.h CommonAPI/DBus/DBusProxy.h 
              PATH_SUFFIXES CommonAPI-0.7
              PATHS
              "/usr/local/include"       	
              "/usr/include")
              
    FIND_LIBRARY(COMMON_API_DBUS_LIBRARY 
             NAMES CommonAPI-DBus
             PATHS
             "/usr/local/lib"
             "/usr/lib"
             NO_SYSTEM_ENVIRONMENT_PATH)
ENDIF(COMMON_API_DBUS_FOUND)
   

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
