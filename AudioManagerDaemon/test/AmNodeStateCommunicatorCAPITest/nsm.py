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
import dbus.mainloop.glib

loop = gobject.MainLoop()
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
 
class NodeStateManager(dbus.service.Object):
  def __init__(self, loop):
    busName = dbus.service.BusName('org.genivi.NodeStateManager.Consumer_org.genivi.NodeStateManager', bus = dbus.SessionBus())
    dbus.service.Object.__init__(self, busName, '/org/genivi/NodeStateManager')
    self.properties = {'RestartReason': 1, 'ShutdownReason': 2, 'WakeUpReason' :3, 'BootMode' :4}
    self.ABus=""
    self.APath=""
    self.loop=loop
 
  @dbus.service.method(dbus_interface='org.freedesktop.DBus.Introspectable', out_signature = 's')
  def Introspect(self):
    f = open('org.genivi.NodeStateManager.Consumer.xml', "r")
    text = f.read()
    return text
 
  @dbus.service.method(dbus_interface='org.genivi.NodeStateManager.Consumer', out_signature = 'ii')
  def GetNodeState(self):
    NodeStateId=1
    ErrorCode=1
    print('[-----nsm-----] send out nodeState' + str(NodeStateId) + ' ErrorCode '+ str(1))
    return NodeStateId, ErrorCode
    
  @dbus.service.method('org.genivi.NodeStateManager.Consumer', out_signature = 'ii')
  def GetApplicationMode(self): 
    ApplicationModeId=5
    ErrorCode=2
    print('[-----nsm-----] send out ApplicationMode' + str(ApplicationModeId) + ' ErrorCode '+ str(2))
    return ApplicationModeId, ErrorCode

  @dbus.service.method('org.genivi.NodeStateManager.Consumer', in_signature='si', out_signature = 'ii')
  def GetSessionState(self,SessionName,seatID): 
    SessionState=0
    ErrorCode=2
    
    if SessionName=="mySession" and seatID==1:
        SessionState=5
        ErrorCode=1
    
    print('[-----nsm-----] GetSessionState for session ' + SessionName + ' seatID '+ str(seatID) + ' returnState ' + str (SessionState))
    return SessionState, ErrorCode
    
  @dbus.service.method('org.genivi.NodeStateManager.Consumer', in_signature='ssuu', out_signature = 'i')
  def RegisterShutdownClient(self,BName,ObjName,ShutdownMode,TimeoutMs): 
    print('[-----nsm-----] Busname: ' + BName)
    print('[-----nsm-----] ObjName: ' + ObjName)
    print('[-----nsm-----] ShutdownMode: ' + str(ShutdownMode))
    print('[-----nsm-----] TimeoutMs: ' + str(TimeoutMs))
    ErrorCode=1
    if TimeoutMs!=100:
        ErrorCode=3
    if BName!="org.genivi.NodeStateManager.LifeCycleConsumer_org.genivi.audiomanager":
        ErrorCode=4
    if ShutdownMode!=1:
        ErrorCode=5
    if ObjName!="/org/genivi/audiomanager":
        ErrorCode=6
    self.ABus=BName
    self.APath=ObjName
    return  ErrorCode
    
  @dbus.service.method('org.genivi.NodeStateManager.Consumer', in_signature='ssu', out_signature = 'i')
  def UnRegisterShutdownClient(self,BusName,ObjName,ShutdownMode): 
    print('[-----nsm-----] Busname: ' + str(BusName))
    print('[-----nsm-----] ObjName: ' + str(ObjName))
    print('[-----nsm-----] ShutdownMode: ' + str(ShutdownMode))
    ErrorCode=1
    if BusName!=self.ABus:
        ErrorCode=2
    if ObjName!=self.APath:
        ErrorCode=2
    if ShutdownMode!=1:
        ErrorCode=2
    return  ErrorCode
    
  @dbus.service.method(dbus_interface='org.genivi.NodeStateManager.Consumer', out_signature = 'u')
  def GetInterfaceVersion(self):
    version=23
    return version
    
  @dbus.service.method('org.genivi.NodeStateManager.Consumer', in_signature='ui', out_signature='i')
  def LifecycleRequestComplete(self,RequestID,Status): 
    print('[-----nsm-----] RequestId: ' + str(RequestID))
    print('[-----nsm-----] Status: ' + str(Status))
    ErrorCode=1
    if RequestID!=22:
        ErrorCode=2
    if Status!=4:
        ErrorCode=2
    return  ErrorCode    

  @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='ss', out_signature='v')
  def Get(self, interface, prop):
    if prop in self.properties:
        print('[-----nsm-----] send out ' + str(self.properties[prop]) + ' for property '+ prop)
        return self.properties[prop]
    return 0
    
  @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='ssv')
  def Set(self, interface, prop, value):
    return 3
    
  @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature='s', out_signature='a{sv}')
  def GetAll(self, interface):
      return self.properties
      
  @dbus.service.signal(dbus_interface='org.genivi.NodeStateManager.Consumer', signature='i')
  def NodeApplicationMode(self, ApplicationModeId):
    print "[-----nsm-----] Send out application mode ID %d" % (ApplicationModeId)
    
  @dbus.service.signal(dbus_interface='org.genivi.NodeStateManager.Consumer', signature='i')
  def NodeState(self, NodeState):
    print "[-----nsm-----] Send out NodeState %d" % (NodeState)
    
  @dbus.service.signal(dbus_interface='org.genivi.NodeStateManager.Consumer', signature='sii')
  def SessionStateChanged(self, SessionStateName,SeatID,SessionState):
    print "[-----nsm-----] Send out SessionStateChanged " + SessionStateName
    
  @dbus.service.method('org.genivi.NodeStateManager.Control', in_signature='i')
  def sendNodeApplicationMode(self, input): 
    self.NodeApplicationMode(input)
    return input
    
  @dbus.service.method('org.genivi.NodeStateManager.Control', in_signature='i')
  def sendNodeState(self, input): 
    self.NodeState(input)
    return input
    
  @dbus.service.method('org.genivi.NodeStateManager.Control', in_signature='sii')
  def sendSessionState(self, SessionStateName,SeatID,SessionState): 
    self.SessionStateChanged (SessionStateName,SeatID,SessionState)
    return SeatID
    
  @dbus.service.method('org.genivi.NodeStateManager.Control', in_signature='uu', out_signature='i')
  def sendLifeCycleRequest(self, request, requestID): 
    bus = dbus.SessionBus()
    remote_object = bus.get_object(self.ABus,self.APath)
    iface = dbus.Interface(remote_object, 'org.genivi.NodeStateManager.LifeCycleConsumer')
    iface.LifecycleRequest(request,requestID)
    return 42
    
  @dbus.service.method('org.genivi.NodeStateManager.Control')
  def finish(self): 
    print '[-----nsm-----] Going to exit now!'
    self.loop.quit()
    return 0

nsm = NodeStateManager(loop)
loop.run()
