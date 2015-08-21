#
# Find common-api sources
#
# This module defines these variables:
#
#  ${PARAMS_TARGET}_GEN_HEADERS
#      A list with generated headers
#  ${PARAMS_TARGET}_GEN_SOURCES
#      A list with generated sources
#  ${PARAMS_TARGET}_GEN_INCLUDE_DIR
#     A list with include directories

include(CMakeParseArguments)


MACRO(LOAD_COMMONAPI)
    #parse the input parameters
    set(options DBUS SOMEIP)
    set(oneValueArgs "")
    set(multiValueArgs "")
    cmake_parse_arguments(PARAMS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(PARAMS_DBUS)
        SET(COMMONAPI_USED_BINDING 0 CACHE INTERNAL "hide this!" FORCE)
    elseif(PARAMS_SOMEIP)
        SET(COMMONAPI_USED_BINDING 1 CACHE INTERNAL "hide this!" FORCE)
    endif()
    
    # load the proper libs ...
	IF(NOT CommonAPI_FOUND)
		FIND_PACKAGE(CommonAPI REQUIRED)
		FIND_LIBRARY(CommonAPI_LIBRARY 
		             REQUIRED
		             NAMES CommonAPI
		             PATHS
		             "/usr/local/lib"
		             "/usr/lib"
		            )  
	ENDIF(NOT CommonAPI_FOUND)
	message(STATUS "CommonAPI Version: ${CommonAPI_VERSION}")
	
	string(REPLACE "." "" COMMONAPI_VERSION_NUMBER ${CommonAPI_VERSION})
	SET(COMMONAPI_VERSION_NUMBER ${COMMONAPI_VERSION_NUMBER} CACHE INTERNAL "hide this!" FORCE)
	SET(CommonAPI_VERSION ${CommonAPI_VERSION} PARENT_SCOPE)    

    IF(${COMMONAPI_USED_BINDING} EQUAL 1)
    	IF(NOT CommonAPI-SomeIP_FOUND)
    	    FIND_PACKAGE (vsomeip REQUIRED)
	        FIND_PACKAGE(CommonAPI-SomeIP REQUIRED)
	        FIND_LIBRARY(CommonAPI-SomeIP_LIBRARY 
	                     REQUIRED
	                     NAMES CommonAPI-SomeIP
	                     PATHS
	                     "/usr/local/lib"
	                     "/usr/lib"
	                    ) 
	                    
        ENDIF(NOT CommonAPI-SomeIP_FOUND)
        message(STATUS "CommonAPI-SomeIP Version: ${CommonAPI-SomeIP_VERSION}")
        
        string(REPLACE "." "" COMMONAPI_SOMEIP_VERSION_NUMBER ${CommonAPI-SomeIP_VERSION})
        SET(COMMONAPI_SOMEIP_VERSION_NUMBER ${COMMONAPI_SOMEIP_VERSION_NUMBER} CACHE INTERNAL "hide this!" FORCE)
    ELSE()
    	SET(COMMONAPI_USED_BINDING 0 CACHE INTERNAL "hide this!" FORCE)
    	IF(NOT CommonAPI-DBus_FOUND)
    		pkg_check_modules (DBUS "dbus-1 >= 1.4" REQUIRED)
	        FIND_PACKAGE(CommonAPI-DBus REQUIRED)
	        FIND_LIBRARY(CommonAPI-DBus_LIBRARY 
	                     REQUIRED
	                     NAMES CommonAPI-DBus
	                     PATHS
	                     "/usr/local/lib"
	                     "/usr/lib"
	                    ) 
        ENDIF(NOT CommonAPI-DBus_FOUND)            
        message(STATUS "CommonAPI-DBus Version: ${CommonAPI-DBus_VERSION}")
        
        string(REPLACE "." "" COMMONAPI_DBUS_VERSION_NUMBER ${CommonAPI-DBus_VERSION})
        SET(COMMONAPI_DBUS_VERSION_NUMBER ${COMMONAPI_DBUS_VERSION_NUMBER} CACHE INTERNAL "hide this!" FORCE)
    ENDIF()
    
ENDMACRO()

# helper function giving a string with the current architecture
function(GET_TARGET_ARCH OUT_ARCH)  
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8) 
        SET(${OUT_ARCH} "x86_64" PARENT_SCOPE)
    ELSE() 
        SET(${OUT_ARCH} "x86" PARENT_SCOPE)
    ENDIF()  
endfunction()

# helper function giving a string with the current host
function(GET_TARGET_HOST OUT_HOST)
    IF(CMAKE_HOST_WIN32)
        SET(${OUT_HOST} "windows" PARENT_SCOPE) 
    ELSE()#CMAKE_HOST_UNIX
        SET(${OUT_HOST} "linux" PARENT_SCOPE)
    ENDIF()
endfunction()

# get lists with headers and sources after they has been generated
macro(GET_GENERATED_FILES GEN_DESTINATION)
       
        #searching for generated headers
        execute_process(COMMAND find ${GEN_DESTINATION} -name *.hpp
                        RESULT_VARIABLE EXIT_CODE
                        OUTPUT_VARIABLE _CAPI_HEADERS
                        ERROR_VARIABLE CAPI_HEADERS_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_STRIP_TRAILING_WHITESPACE)
        
        IF(CAPI_HEADERS_ERROR)
            MESSAGE(FATAL_ERROR "Can't find common-api generated headers!")
        ENDIF()
        string(REPLACE "\n" ";" CAPI_HEADERS ${_CAPI_HEADERS})
        
        FOREACH(INCLUDE_ITER ${CAPI_HEADERS})
            GET_FILENAME_COMPONENT(TEMP_PATH ${INCLUDE_ITER} PATH)
            SET(CAPI_INCLUDES ${CAPI_INCLUDES} ${TEMP_PATH})
        ENDFOREACH(INCLUDE_ITER ${CAPI_HEADERS})       
        LIST(REMOVE_DUPLICATES CAPI_INCLUDES)
        
        #searching for generated sources
        execute_process(COMMAND find ${GEN_DESTINATION} -name *.cpp
                        RESULT_VARIABLE EXIT_CODE
                        OUTPUT_VARIABLE _CAPI_SOURCES
                        ERROR_VARIABLE CAPI_SOURCES_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_STRIP_TRAILING_WHITESPACE)
        IF(CAPI_SOURCES_ERROR)
            MESSAGE(FATAL_ERROR "Can't find common-api generated sources!")
        ENDIF()
        string(REPLACE "\n" ";" CAPI_SOURCES ${_CAPI_SOURCES})
        LIST(REMOVE_DUPLICATES CAPI_SOURCES)
                           
        set(${PARAMS_TARGET}_GEN_HEADERS ${CAPI_HEADERS} PARENT_SCOPE)
        set(${PARAMS_TARGET}_GEN_SOURCES ${CAPI_SOURCES} PARENT_SCOPE)        
        #add base path src-gen
        SET(${PARAMS_TARGET}_GEN_INCLUDE_DIR ${CAPI_INCLUDES} ${GEN_DESTINATION} PARENT_SCOPE)   
endmacro(GET_GENERATED_FILES)

macro(FIND_AND_EXEC_GENERATOR GENERATOR_EXECUTABLE SHOULD_GENERATE_STUB_DEFAULT FIDLS)
    MESSAGE(STATUS "Searching for common-api generator executable ${GENERATOR_EXECUTABLE} ...")
    # find the generator binary ...
    execute_process(COMMAND find "/usr/local/share/CommonAPI-${CommonAPI_VERSION}" -name ${GENERATOR_EXECUTABLE}
                        RESULT_VARIABLE EXIT_CODE
                        OUTPUT_VARIABLE OUT_RESULT
                        ERROR_VARIABLE OUT_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_STRIP_TRAILING_WHITESPACE)
	
    IF(OUT_ERROR)
         # in case of error just try to find the sources in the alternative folder
        SET(TEMP_GEN_DST ${PARAMS_ALT_DESTINATION})
        message(STATUS "Common-api generator error / ${OUT_ERROR} /. Will try the alternative folder!")
    ELSEIF(NOT OUT_RESULT)
        # in case of error just try to find the sources in the alternative folder
        SET(TEMP_GEN_DST ${PARAMS_ALT_DESTINATION})
        message(STATUS "Common-api generator can't be found. Will try the alternative folder!")
    ELSE()
        # the generator binary is found
        MESSAGE(STATUS "Will execute common-api generator at path ${OUT_RESULT} with ${FIDLS}")
        function(mktmpdir OUTVAR)
            while(NOT TEMP_DESTINATION OR EXISTS ${TEMP_DESTINATION})
                string(RANDOM LENGTH 16 TEMP_DESTINATION)
                set(TEMP_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DESTINATION}")
            endwhile()

           file(MAKE_DIRECTORY ${TEMP_DESTINATION})

           set(${OUTVAR} ${TEMP_DESTINATION} PARENT_SCOPE)
        endfunction()
        # execute the generate command ...     
        IF(${SHOULD_GENERATE_STUB_DEFAULT} EQUAL 1)  
            execute_process(COMMAND ${OUT_RESULT} -sk Default -d ${PARAMS_DESTINATION} ${FIDLS}
	                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                        RESULT_VARIABLE EXIT_CODE
	                        OUTPUT_VARIABLE GENERATOR_OUTPUT
	                        ERROR_VARIABLE GENERATOR_ERR_OUTPUT
	                        OUTPUT_STRIP_TRAILING_WHITESPACE
	                        ERROR_STRIP_TRAILING_WHITESPACE)
        ELSE()
      		execute_process(COMMAND ${OUT_RESULT} -d ${PARAMS_DESTINATION} ${FIDLS}
	                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                        RESULT_VARIABLE EXIT_CODE
	                        OUTPUT_VARIABLE GENERATOR_OUTPUT
	                        ERROR_VARIABLE GENERATOR_ERR_OUTPUT
	                        OUTPUT_STRIP_TRAILING_WHITESPACE
	                        ERROR_STRIP_TRAILING_WHITESPACE)
        ENDIF()                
        if(EXIT_CODE)
            message(FATAL_ERROR "Failed to generate files from FIDL:${GENERATOR_OUTPUT}")
        elseif(GENERATOR_ERR_OUTPUT)
        	message(FATAL_ERROR "Common-API generator error:${GENERATOR_ERR_OUTPUT}")
        endif()
        SET(TEMP_GEN_DST ${PARAMS_DESTINATION})   
    ENDIF()
endmacro(FIND_AND_EXEC_GENERATOR GENERATOR_EXECUTABLE SHOULD_GENERATE_STUB_DEFAULT FIDLS)

# generate common-api sources and retreive a list with them 
MACRO(EXECUTE_GENERATOR)    
    # construct the generator binary name...
    GET_TARGET_HOST(_TARGET_HOST)
    GET_TARGET_ARCH(_TARGET_ARCH)
    SET(COMMONAPI_GENERATOR_EXECUTABLE commonapi-generator-${_TARGET_HOST}-${_TARGET_ARCH})
    IF(${COMMONAPI_USED_BINDING} EQUAL 1)
        SET(COMMONAPI_BINDING_GENERATOR_EXECUTABLE commonapi-someip-generator-${_TARGET_HOST}-${_TARGET_ARCH})
    ELSE()
        SET(COMMONAPI_BINDING_GENERATOR_EXECUTABLE commonapi-dbus-generator-${_TARGET_HOST}-${_TARGET_ARCH})
    ENDIF()
    # prepare an additional compatibilty flag for generators prior 3.x.x ...
    IF(${COMMONAPI_VERSION_NUMBER} GREATER 300 OR ${COMMONAPI_VERSION_NUMBER} EQUAL 300)
        # >= 3.x.x the stubs are generated by the binding generator
       	SET(GENERATE_STUB 1) 
    ELSE() 
        # < 3.0.0 the stubs are generated by the genric generator
    	SET(GENERATE_STUB 0) 
    ENDIF()
    # searching for common-api-generator executable ...
    FOREACH(FIDL ${IN_FIDLS_GENERIC})
    	FIND_AND_EXEC_GENERATOR(${COMMONAPI_GENERATOR_EXECUTABLE} ${GENERATE_STUB} ${FIDL})
    ENDFOREACH()
    FOREACH(FIDL ${IN_FIDLS_BINDING})
    	FIND_AND_EXEC_GENERATOR(${COMMONAPI_BINDING_GENERATOR_EXECUTABLE} FALSE ${FIDL})
    ENDFOREACH()
    # get the lists with the sources and headers
    message(STATUS "Looking for generated common-api files...")
    GET_GENERATED_FILES(${TEMP_GEN_DST})
ENDMACRO(EXECUTE_GENERATOR)

# Function COMMON_API_GENERATE_SOUCRES 
#
# TARGET COMMON_API 
# FIDLS_GENERIC a list with fidls for the generic generator.
# FIDLS_BINDING a list with fidls for the binding generator.
# DESTINATION a relative path to the build directory or an absolute path.                                
# ALT_DESTINATION an alternative relative/absolute path with common-api sources, usually in the source tree.
FUNCTION(COMMON_API_GENERATE_SOURCES)
    #parse the input parameters
    set(options "")
    set(oneValueArgs TARGET DESTINATION ALT_DESTINATION HEADER_TEMPLATE)
    set(multiValueArgs FIDLS_GENERIC FIDLS_BINDING)

    cmake_parse_arguments(PARAMS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT PARAMS_TARGET)
           message(FATAL_ERROR "TARGET must be specified")
    endif()
    
    if(NOT IS_ABSOLUTE ${PARAMS_DESTINATION})
         set(PARAMS_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${PARAMS_DESTINATION})
    endif()
    
    if(NOT IS_ABSOLUTE ${PARAMS_ALT_DESTINATION})
         set(PARAMS_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${PARAMS_ALT_DESTINATION})
    endif()
    # check the cmake option, whether to use the generator or not ...
    IF(NOT WITH_COMMON_API_GEN)
        message(STATUS "Looking for generated common-api files...")
        # check which of the given folders exists and get it as destination
        IF(EXISTS ${PARAMS_DESTINATION})
            GET_GENERATED_FILES(${PARAMS_DESTINATION})
        ELSE()
            GET_GENERATED_FILES(${PARAMS_ALT_DESTINATION})
        ENDIF()    
    ELSE()
        if(NOT PARAMS_FIDLS_GENERIC)
            message(FATAL_ERROR "FIDLS must be specified")
        endif()
    
        if(PARAMS_HEADER_TEMPLATE)
            list(APPEND ARGS -pref ${PARAMS_HEADER_TEMPLATE})
        endif()
        
        # Run configure_file on each .fidl which forces cmake to reexecute its configure phase if the input file changes.
        foreach(FIDL ${PARAMS_FIDLS_GENERIC})
            get_filename_component(FIDL_PATH ${FIDL} ABSOLUTE)
            string(MD5 ${FIDL_PATH} FIDL_CHECKSUM)
            configure_file(${FIDL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${FIDL_CHECKSUM}.fidl.done)
            list(APPEND IN_FIDLS_GENERIC ${FIDL_PATH})
        endforeach()

        if(PARAMS_FIDLS_BINDING)
            foreach(FIDL ${PARAMS_FIDLS_BINDING})
                get_filename_component(FIDL_PATH ${FIDL} ABSOLUTE)
                string(MD5 ${FIDL_PATH} FIDL_CHECKSUM)
                configure_file(${FIDL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${FIDL_CHECKSUM}.fidl.done)
                list(APPEND IN_FIDLS_BINDING ${FIDL_PATH})
            endforeach()
        else()
            SET(IN_FIDLS_BINDING ${IN_FIDLS_GENERIC})
        endif()
        
        # run the generator ...
        EXECUTE_GENERATOR()
    ENDIF()
ENDFUNCTION()

