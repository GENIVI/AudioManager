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

pkg_check_modules(COMMON_API CommonAPI)

IF(COMMON_API_FOUND)	
    
    FIND_PATH(COMMON_API_INCLUDE_DIR
              NAMES CommonAPI/Runtime.h CommonAPI/Proxy.h 
              PATH_SUFFIXES CommonAPI-${COMMON_API_VERSION}	 
              PATHS
              ${COMMON_API_INCLUDE_DIRS}	
              /usr/local/include        	
              /usr/include)

    FIND_LIBRARY(COMMON_API_LIBRARY 
                 NAMES CommonAPI
                 PATHS
                 ${CommonAPI_PKG_LIBRARY_DIRS}
                 /usr/local/lib
                 /usr/lib)
                 
ELSE(COMMON_API_FOUND)
    
    MESSAGE(STATUS "CommonAPI package not found, search directly, trying version 0.7 ...")

      
    FIND_PATH(COMMON_API_INCLUDE_DIR
              NAMES CommonAPI/Runtime.h CommonAPI/Proxy.h 
              PATH_SUFFIXES CommonAPI-0.7
              PATHS
              /usr/local/include        	
              /usr/include)
              
    FIND_LIBRARY(COMMON_API_LIBRARY 
             NAMES CommonAPI
             PATHS
             /usr/local/lib
             /usr/lib)
              
ENDIF(COMMON_API_FOUND)
	
SET(COMMON_API_LIBRARIES ${COMMON_API_LIBRARY})

IF(COMMON_API_INCLUDE_DIR AND COMMON_API_LIBRARY)   
   message(STATUS "Found CommonAPI")
ELSE(COMMON_API_LIBRARIES AND COMMON_API_LIBRARY)   
   message(STATUS " CommonAPI not found.")
ENDIF(COMMON_API_INCLUDE_DIR AND COMMON_API_LIBRARY)

#searching for generated headers
IF(NOT COMMON_API_SRC_GEN)
     SET(COMMON_API_SRC_GEN "src-gen/")
ENDIF(NOT COMMON_API_SRC_GEN)

FILE(GLOB_RECURSE COMMON_API_GEN_HEADER_DIRECTORIES "${COMMON_API_SRC_GEN}*Proxy.h")
FOREACH(INCLUDE_ITER ${COMMON_API_GEN_HEADER_DIRECTORIES})
   GET_FILENAME_COMPONENT(TEMP_PATH ${INCLUDE_ITER} PATH)
   SET(COMMON_API_GEN_INCLUDE_DIR ${COMMON_API_GEN_INCLUDE_DIR} ${TEMP_PATH})
ENDFOREACH(INCLUDE_ITER ${COMMON_API_GEN_HEADER_DIRECTORIES})
LIST(REMOVE_DUPLICATES COMMON_API_GEN_INCLUDE_DIR)

#add base path src-gen
SET(COMMON_API_GEN_INCLUDE_DIR ${COMMON_API_GEN_INCLUDE_DIR} ${COMMON_API_SRC_GEN})

IF (COMMON_API_GEN_INCLUDE_DIR)
    message(STATUS "Found generated headers !")	
ELSE (COMMON_API_GEN_INCLUDE_DIR)
    message(STATUS "Did not find generated headers")
ENDIF(COMMON_API_GEN_INCLUDE_DIR)

#searching for generated sources
FILE(GLOB_RECURSE COMMON_API_GEN_SOURCES "${COMMON_API_SRC_GEN}*.cpp")

IF (COMMON_API_GEN_SOURCES)
    message(STATUS "Found generated sources !")	
ELSE (COMMON_API_GEN_SOURCES)
    message(STATUS "Did not find generated sources !")
ENDIF(COMMON_API_GEN_SOURCES)
		
MARK_AS_ADVANCED(
    COMMON_API_LIBRARIES 
    COMMON_API_INCLUDE_DIR
    COMMON_API_GEN_INCLUDE_DIR
    COMMON_API_GEN_SOURCES
)
