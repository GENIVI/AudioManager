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
                 
ELSE(COMMON_API_FOUND)
    
    MESSAGE(STATUS "CommonAPI package not found, search directly, trying version 0.7 ...")

      
    FIND_PATH(COMMON_API_INCLUDE_DIR
              NAMES CommonAPI/Runtime.h CommonAPI/Proxy.h 
              PATH_SUFFIXES CommonAPI-0.7
              PATHS
              "/usr/local/include"       	
              "/usr/include"
              )
              
    FIND_LIBRARY(COMMON_API_LIBRARY 
             NAMES CommonAPI
             PATHS
             "/usr/local/lib"
             "/usr/lib"
             )
              
ENDIF(COMMON_API_FOUND)

SET(COMMON_API_LIBRARIES ${COMMON_API_LIBRARY})

IF(COMMON_API_INCLUDE_DIR AND COMMON_API_LIBRARY)   
   message(STATUS "Found CommonAPI")
ELSE(COMMON_API_LIBRARIES AND COMMON_API_LIBRARY)   
   message(STATUS " CommonAPI not found.")
ENDIF(COMMON_API_INCLUDE_DIR AND COMMON_API_LIBRARY)

#searching for generated headers
FILE(GLOB_RECURSE COMMON_API_GEN_HEADER_DIRECTORIES "src-gen/*.h")
FOREACH(INCLUDE_ITER ${COMMON_API_GEN_HEADER_DIRECTORIES})
   GET_FILENAME_COMPONENT(TEMP_PATH ${INCLUDE_ITER} PATH)
   SET(COMMON_API_GEN_INCLUDE_DIR ${COMMON_API_GEN_INCLUDE_DIR} ${TEMP_PATH})
ENDFOREACH(INCLUDE_ITER ${COMMON_API_GEN_HEADER_DIRECTORIES})
LIST(REMOVE_DUPLICATES COMMON_API_GEN_INCLUDE_DIR)

#add base path src-gen
SET(COMMON_API_GEN_INCLUDE_DIR ${COMMON_API_GEN_INCLUDE_DIR} "src-gen/")

IF (COMMON_API_GEN_INCLUDE_DIR)
    message(STATUS "Found generated headers !")	
ELSE (COMMON_API_GEN_INCLUDE_DIR)
    message(STATUS "Did not find generated headers")
ENDIF(COMMON_API_GEN_INCLUDE_DIR)

#searching for generated sources
FILE(GLOB_RECURSE COMMON_API_GEN_SOURCES "src-gen/*.cpp")

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
