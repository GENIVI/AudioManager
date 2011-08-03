/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * PluginDBus
 *
 * \file RoutingSend.cpp
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 */


#include "headers.h"
#include "routinginterface.h"
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>

DLT_DECLARE_CONTEXT(DBusPlugin)

DbusInterface::DbusInterface(): m_busname(DBUS_BUSNAME), m_path(DBUS_PATH) {

	DLT_REGISTER_APP("DBusPlugin", "DBusPlugin");
	DLT_REGISTER_CONTEXT(DBusPlugin, "DPlugin", "Dbus Plugin");
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("The DBus Plugin is started"));
}

DbusInterface::~DbusInterface() {
	delete m_DbusInterface;
}

void DbusInterface::startup_interface(RoutingReceiveInterface* audioman,dbusRoothandler* dbushandler) {
	m_audioman = audioman;
	m_rootHandler = dbushandler;
	m_DbusInterface = new AudioManagerInterface(audioman,dbushandler);
	m_DbusInterface->startup_interface();
	m_rootHandler->registerNode(MY_NODE);
	DBusError err;
    dbus_error_init(&err);

    m_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Could not connect to DBUS for sending, Error: "), DLT_STRING(err.message));
	    dbus_error_free(&err);
	}

	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("DBus Interface started "));
}

void DbusInterface::return_BusName(char* BusName) {
	strcpy(BusName, BUS_NAME);
}

genError_t DbusInterface::connect(source_t source, sink_t sink, connection_t connID) {
	int reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path, "connect");
	send.appendInteger(source);
	send.appendInteger(sink);
	send.appendInteger(connID);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

void DbusInterface::system_ready() {
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("DBus Plugin got ready"));
	m_DbusInterface->emit_systemReady();
}

genError_t DbusInterface::disconnect(connection_t connectionID) {
	bool reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"disconnect");
	send.appendInteger(connectionID);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

genError_t DbusInterface::setSinkVolume(volume_t volume, sink_t sink) {
	int reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"setSinkVolume");
	send.appendInteger(volume);
	send.appendInteger(sink);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

genError_t DbusInterface::setSourceVolume(volume_t volume, source_t source) {
	int reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"setSourceVolume");
	send.appendInteger(volume);
	send.appendInteger(source);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

genError_t DbusInterface::muteSource(source_t sourceID) {
	bool reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"muteSource");
	send.appendInteger(sourceID);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

genError_t DbusInterface::muteSink(sink_t sinkID) {
	bool reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"muteSink");
	send.appendInteger(sinkID);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

genError_t DbusInterface::unmuteSource(source_t sourceID) {
	bool reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"unmuteSource");
	send.appendInteger(sourceID);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

genError_t DbusInterface::unmuteSink(sink_t sinkID) {
	bool reply;
	DbusSend send = DbusSend(m_conn,"PULSE",(const char*)m_busname,(const char*)m_path,"unmuteSink");
	send.appendInteger(sinkID);
	send.sendReply(&reply);
	return GEN_OK;
	/**
	 * \todo always OK...
	 */
}

//genError_t DbusInterface::asyncConnect(source_t source, sink_t sink, connection_t con_ID) {
//	/**
//	 * \todo implement
//	 */
//	return GEN_OK;
//}
//
//genError_t DbusInterface::asyncDisconnect(connection_t connection_ID){
//	/**
//	 * \todo implement
//	 */
//	return GEN_OK;
//}

//That is the actual implementation of the Factory Class returning the real sendInterface

extern "C" RoutingSendInterface* PluginRoutingInterfaceDbusFactory() {
    return new DbusInterface();
}

extern "C" void destroyRoutingPluginInterfaceDbus(RoutingSendInterface* iface) {
    delete iface;
}


