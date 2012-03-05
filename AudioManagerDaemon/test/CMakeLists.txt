# Copyright (C) 2012, GENIVI Alliance, Inc.
# Copyright (C) 2012, BMW AG
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
# author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 2.6)


set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})
FIND_PACKAGE(GTest REQUIRED) 

add_subdirectory (AmControlInterfaceTest)
add_subdirectory (AmDatabaseHandlerTest)
add_subdirectory (AmRouterTest)
add_subdirectory (AmRoutingInterfaceTest)
add_subdirectory (AmSocketHandlerTest)
add_subdirectory (AmTelnetServerTest)
