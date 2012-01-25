 #
 #  Copyright (C) 2011, BMW AG 
 # 
 # AudioManagerDeamon Test
 #
 # @file CMakeLists.txt
 #
 # @date: 20.05.2011
 # @author: Christian Müller (christian.ei.mueller@bmw.de)
 #

cmake_minimum_required(VERSION 2.6)


set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})
FIND_PACKAGE(GTest REQUIRED) 

add_subdirectory (database)
add_subdirectory (routing)
add_subdirectory (routingInterface)
add_subdirectory (controlInterface)
add_subdirectory (sockethandler)
