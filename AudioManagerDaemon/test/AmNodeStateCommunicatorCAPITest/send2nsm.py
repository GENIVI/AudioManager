# -*- coding: utf-8 -*-
#
# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
#
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
#
# \copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#
# \author Christian Linke, christian.linke@bmw.de BMW 2012
#
# For further information see http://www.genivi.org/.
#

import sys
import traceback
import gobject
import math
import dbus
import dbus.service

def nodeState (nodeState):
	bus = dbus.SessionBus()
	remote_object = bus.get_object('org.genivi.NodeStateManager.Consumer_org.genivi.NodeStateManager','/org/genivi/NodeStateManager')
	iface = dbus.Interface(remote_object, 'org.genivi.NodeStateManager.Control')
	iface.sendNodeState(int(nodeState))
	
def appMode (appMode):
	bus = dbus.SessionBus()
	remote_object = bus.get_object('org.genivi.NodeStateManager.Consumer_org.genivi.NodeStateManager','/org/genivi/NodeStateManager')
	iface = dbus.Interface(remote_object, 'org.genivi.NodeStateManager.Control')
	iface.sendNodeApplicationMode(int(appMode))
	
def sessionState (SessionStateName,SeatID,SessionState):
	bus = dbus.SessionBus()
	remote_object = bus.get_object('org.genivi.NodeStateManager.Consumer_org.genivi.NodeStateManager','/org/genivi/NodeStateManager')
	iface = dbus.Interface(remote_object, 'org.genivi.NodeStateManager.Control')
	iface.sendSessionState(SessionStateName,int(SeatID),int(SessionState))
	
def finish():
	bus = dbus.SessionBus()
	remote_object = bus.get_object('org.genivi.NodeStateManager.Consumer_org.genivi.NodeStateManager','/org/genivi/NodeStateManager')
	iface = dbus.Interface(remote_object, 'org.genivi.NodeStateManager.Control')
	iface.finish()

def LifecycleRequest(Request,RequestID):
	bus = dbus.SessionBus()
	remote_object = bus.get_object('org.genivi.NodeStateManager.Consumer_org.genivi.NodeStateManager','/org/genivi/NodeStateManager')
	iface = dbus.Interface(remote_object, 'org.genivi.NodeStateManager.Control')
	iface.sendLifeCycleRequest(dbus.UInt32(Request),dbus.UInt32(RequestID))

command=sys.argv[1]
if command=="nodeState":
	nodeState(sys.argv[2])	
if command=="finish":
	finish()
if command=="appMode":
	appMode(sys.argv[2])
if command=="sessionState":
	sessionState(sys.argv[2],sys.argv[3],sys.argv[4])
if command=="LifecycleRequest":
	LifecycleRequest(sys.argv[2],sys.argv[3])
