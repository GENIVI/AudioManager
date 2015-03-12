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

IF(COMMON_API_FOUND AND COMMON_API_DBUS_FOUND)
    # get lists with headers and sources after they has been generated
    macro(SEARCH_FOR_COMMON_API_GEN_FILES GEN_DESTINATION)
           
            #searching for generated headers
            execute_process(COMMAND find ${GEN_DESTINATION} -name *.h
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
    endmacro()
    
    # generate common-api sources and retreive a list with them 
    macro(GENERATE_FILES)    


        # searching for common-api-generator executable ...
        FIND_PATH(COMMONAPI_GENERATOR_JAR
                      NAMES 
                          "common-api-generator.jar"
                      PATH_SUFFIXES 
                          CommonAPI-${COMMON_API_VERSION}	 
                      PATHS
                          "/usr/share" 
                          "/usr/local/share")
       
		FIND_PATH(COMMONAPI_GENERATOR_EXE
                      NAMES 
                          "commonapi_generator"
                      PATH_SUFFIXES 
                          CommonAPI-${COMMON_API_VERSION}	 
                      PATHS
						  "~"
                          "/usr/share" 
                          "/usr/local/share")

        if(COMMONAPI_GENERATOR_JAR)
            # load java runtime ...
            find_package(Java COMPONENTS Runtime REQUIRED QUIET)
            function(mktmpdir OUTVAR)
                while(NOT TEMP_DESTINATION OR EXISTS ${TEMP_DESTINATION})
                    string(RANDOM LENGTH 16 TEMP_DESTINATION)
                    set(TEMP_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DESTINATION}")
                endwhile()
    
               file(MAKE_DIRECTORY ${TEMP_DESTINATION})
    
               set(${OUTVAR} ${TEMP_DESTINATION} PARENT_SCOPE)
            endfunction()
            # execute the generate command ...
            execute_process(COMMAND ${Java_JAVA_EXECUTABLE} -jar "${COMMONAPI_GENERATOR_JAR}/common-api-generator.jar" -dest ${PARAMS_DESTINATION} ${ARGS} ${FIDLS}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE EXIT_CODE
                            OUTPUT_VARIABLE GENERATOR_OUTPUT
                            ERROR_VARIABLE GENERATOR_OUTPUT
                            OUTPUT_STRIP_TRAILING_WHITESPACE
                            ERROR_STRIP_TRAILING_WHITESPACE)
            if(EXIT_CODE)
                message(FATAL_ERROR "Failed to generate files from FIDL:\n ${GENERATOR_OUTPUT}")
            endif()
            SET(TEMP_GEN_DST ${PARAMS_DESTINATION})
		elseif(COMMONAPI_GENERATOR_EXE)
            # load executable
            function(mktmpdir OUTVAR)
                while(NOT TEMP_DESTINATION OR EXISTS ${TEMP_DESTINATION})
                    string(RANDOM LENGTH 16 TEMP_DESTINATION)
                    set(TEMP_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DESTINATION}")
                endwhile()
    
               file(MAKE_DIRECTORY ${TEMP_DESTINATION})
    
               set(${OUTVAR} ${TEMP_DESTINATION} PARENT_SCOPE)
            endfunction()
    
            # execute the generate command ...
            execute_process(COMMAND ${COMMONAPI_GENERATOR_EXE}/commonapi_generator -dest ${PARAMS_DESTINATION} ${ARGS} ${FIDLS}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                            RESULT_VARIABLE EXIT_CODE
                            OUTPUT_VARIABLE GENERATOR_OUTPUT
                            ERROR_VARIABLE GENERATOR_OUTPUT
                            OUTPUT_STRIP_TRAILING_WHITESPACE
                            ERROR_STRIP_TRAILING_WHITESPACE)
            if(EXIT_CODE)
                message(FATAL_ERROR "Failed to generate files from FIDL:\n ${GENERATOR_OUTPUT}")
            endif()
            SET(TEMP_GEN_DST ${PARAMS_DESTINATION})	
        else()
            # if the generator is not found, try to find the sources in the alternative folder
            SET(TEMP_GEN_DST ${PARAMS_ALT_DESTINATION})
            message(STATUS "Couldn't find a common-api generator...skiping generation!")
        endif()
        # get the lists with the sources and headers
        message(STATUS "Looking for available common-api generated files...")
        SEARCH_FOR_COMMON_API_GEN_FILES(${TEMP_GEN_DST})
    endmacro()
    
    function(COMMON_API_GENERATE_SOUCRES)
        #parse the input parameters
        set(options DBUS)
        set(oneValueArgs TARGET DESTINATION ALT_DESTINATION HEADER_TEMPLATE)
        set(multiValueArgs FIDLS FIDL_DEPENDS)

        cmake_parse_arguments(PARAMS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
        
         if(NOT IS_ABSOLUTE ${PARAMS_DESTINATION})
             set(PARAMS_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${PARAMS_DESTINATION})
        endif()
        
        if(NOT IS_ABSOLUTE ${PARAMS_ALT_DESTINATION})
             set(PARAMS_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${PARAMS_ALT_DESTINATION})
        endif()
        # check the cmake option, whether to use the generator or not ...
        IF(NOT WITH_COMMON_API_GEN)
            message(STATUS "Looking for available common-api generated files...")
            # check which of the given folders exists and get it as destination
            IF(EXISTS ${PARAMS_DESTINATION})
                SEARCH_FOR_COMMON_API_GEN_FILES(${PARAMS_DESTINATION})
            ELSE()
                SEARCH_FOR_COMMON_API_GEN_FILES(${PARAMS_ALT_DESTINATION})
            ENDIF()    
        ELSE()
            message(STATUS "Will generate common-api files...")
            if(NOT PARAMS_FIDLS)
                message(FATAL_ERROR "FIDLS must be specified")
            endif()
        
            if(NOT PARAMS_TARGET)
                message(FATAL_ERROR "TARGET must be specified")
            endif()
        
            if(PARAMS_DBUS)
                  list(APPEND ARGS -dbus)
            endif()
        
            if(PARAMS_HEADER_TEMPLATE)
                list(APPEND ARGS -pref ${PARAMS_HEADER_TEMPLATE})
            endif()
            
            foreach(FIDL ${PARAMS_FIDLS})
                get_filename_component(FIDL_PATH ${FIDL} ABSOLUTE)
            
                # Run configure_file on the .fidl - this forces cmake to reexecute its
                # configure phase if the input file changes.
                string(MD5 ${FIDL_PATH} FIDL_CHECKSUM)
                configure_file(${FIDL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${FIDL_CHECKSUM}.fidl.done)
            
                list(APPEND FIDLS ${FIDL_PATH})
            endforeach()
            
            message(STATUS "Determining list of generated files for ${PARAMS_FIDLS}")
            
            foreach(FIDL_DEPEND ${PARAMS_FIDL_DEPENDS})
                string(MD5 ${FIDL_PATH} FIDL_CHECKSUM)
                configure_file(${FIDL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${FIDL_CHECKSUM}.fidl.done)
            endforeach()
            GENERATE_FILES()
        ENDIF()
    endfunction()

ENDIF(COMMON_API_FOUND AND COMMON_API_DBUS_FOUND)
