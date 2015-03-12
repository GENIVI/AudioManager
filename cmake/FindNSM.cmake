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

FIND_PATH (NSM_INCLUDE_DIR NodeStateManager.h
             PATHS
             ${CMAKE_INSTALL_PATH}
             "/usr/include"
             "/usr/local/include"
             DOC "The nodestatemanager include directory")

if(NSM_INCLUDE_DIR)
    set (NSM_FOUND "YES")
    message(STATUS "Found NSM include: ${NSM_INCLUDE_DIR}")
else(NSM_INCLUDE_DIR)
    set (NSM_FOUND "YES")
    set (NSM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/nodeStateManagerIncludes")
    message(STATUS "Did not find NSM include, using own include dir: ${NSM_INCLUDE_DIR}")
endif(NSM_INCLUDE_DIR)

    mark_as_advanced(NSM_INCLUDE_DIR)