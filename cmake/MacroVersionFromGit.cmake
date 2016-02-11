# get the current git version
execute_process(COMMAND git describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE DAEMONVERSION 
                OUTPUT_STRIP_TRAILING_WHITESPACE)           

if (NOT DAEMONVERSION)
    #Can be changed via passing -DVERSION="XXX" to cmake
    if(NOT DEFINED VERSION)
        set( DAEMONVERSION "homebrew-${CMAKE_SOURCE_DIR}" )
    else (NOT DEFINED VERSION)
        set( DAEMONVERSION "${VERSION}" )   
    endif(NOT DEFINED VERSION)
else (NOT DAEMONVERSION)
    STRING(REGEX REPLACE "(-)[^-]+$" "" DAEMONVERSION ${DAEMONVERSION})
    STRING(REGEX REPLACE "-" "." DAEMONVERSION ${DAEMONVERSION})
endif(NOT DAEMONVERSION)

message(STATUS "Build Version ${DAEMONVERSION}")

execute_process(COMMAND git log --pretty=short WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG)

