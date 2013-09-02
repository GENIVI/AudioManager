# Copyright (C) 2013, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Linke, christian.linke@bmw.de BMW 2013
#
# For further information see http://www.genivi.org/.
#

include(UsePkgConfig)

pkg_check_modules(COMMON_API_DBUS CommonAPI-DBus)

IF(COMMON_API_DBUS_FOUND)	
    
    FIND_PATH(COMMON_API_DBUS_INCLUDE_DIR
              NAMES CommonAPI/DBus/DBusRuntime.h CommonAPI/DBus/DBusProxy.h 
              PATH_SUFFIXES CommonAPI-${COMMON_API_VERSION}	 
              PATHS
              ${COMMON_API_DBUS_INCLUDE_DIRS}	
              /usr/local/include        	
              /usr/include)

    FIND_LIBRARY(COMMON_API_DBUS_LIBRARY 
                 NAMES CommonAPI-DBus 
                 PATHS
                 ${CommonAPI_PKG_LIBRARY_DIRS}
                 /usr/local/lib
                 /usr/lib)
                 
                                                                
ELSE(COMMON_API_DBUS_FOUND)
    
    MESSAGE(STATUS "CommonAPI_DBUS package not found, search directly, trying version 0.7 ...")

      
    FIND_PATH(COMMON_API_DBUS_INCLUDE_DIR
              NAMES CommonAPI/DBus/DBusRuntime.h CommonAPI/DBus/DBusProxy.h 
              PATH_SUFFIXES CommonAPI-0.7
              PATHS
              /usr/local/include        	
              /usr/include)
              
    FIND_LIBRARY(COMMON_API_DBUS_LIBRARY 
             NAMES CommonAPI-DBus
             PATHS
             /usr/local/lib
             /usr/lib)
             
         
ENDIF(COMMON_API_DBUS_FOUND)

FIND_LIBRARY(DBUS_LIBRARY
    NAMES dbus-1
    PATHS 
    /lib
    /usr/local/lib
    /usr/lib
)

IF (NOT DBUS_LIBRARY)
    MESSAGE (ERROR "did not find DBus library!")
ENDIF (NOT DBUS_LIBRARY)
    

SET(COMMON_API_DBUS_LIBRARIES ${COMMON_API_DBUS_LIBRARY} ${DBUS_LIBRARY})

IF(COMMON_API_DBUS_INCLUDE_DIR AND COMMON_API_DBUS_LIBRARY)   
   message(STATUS "Found CommonAPI_DBUS")
ELSE(COMMON_API_DBUS_INCLUDE_DIR AND COMMON_API_DBUS_LIBRARY)   
   message(STATUS " CommonAPI_DBUS not found.")
ENDIF(COMMON_API_DBUS_INCLUDE_DIR AND COMMON_API_DBUS_LIBRARY)
		
MARK_AS_ADVANCED(
    COMMON_API_DBUS_LIBRARIES 
    COMMON_API_DBUS_INCLUDE_DIR
)
