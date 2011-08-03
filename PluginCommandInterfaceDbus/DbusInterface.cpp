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

DLT_DECLARE_CONTEXT(DBusCommandPlugin)

DbusCommandInterface::DbusCommandInterface() : m_busname(DBUS_BUSNAME), m_path(DBUS_PATH) {

	DLT_REGISTER_APP("DBusCommandPlugin", "DBusCommandPlugin");
	DLT_REGISTER_CONTEXT(DBusCommandPlugin, "PluginCommand", "DBusCommandPlugin");
	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("The DBusCommandPluginis started"));
}

DbusCommandInterface::~DbusCommandInterface() {
	delete m_DbusInterface;
}

void DbusCommandInterface::startupInterface(CommandReceiveInterface* iface) {
	m_audioman = iface;
	m_DbusInterface = new CommandDbusReceive(iface);
	m_DbusInterface->startup_interface();

	DBusError err;
    dbus_error_init(&err);

    m_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Could not connect to DBUS for sending, Error: "), DLT_STRING(err.message));
	    dbus_error_free(&err);
	}

	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("DBus Interface started "));
}



void DbusCommandInterface::stop(){
}



void DbusCommandInterface::cbConnectionChanged(){
	m_DbusInterface->emitSignalConnectionsChanged();
}



void DbusCommandInterface::cbNumberOfSinksChanged(){
	m_DbusInterface->emitSignalNumberofSinksChanged();
}



void DbusCommandInterface::cbNumberOfSourcesChanged(){
	m_DbusInterface->emitSignalNumberofSourcesChanged();
}


//That is the actual implementation of the Factory Class returning the real sendInterface

extern "C" CommandSendInterface* PluginCommandInterfaceDbusFactory() {
    return new DbusCommandInterface();
}

extern "C" void destroyRoutingPluginInterfaceDbus(DbusCommandInterface* iface) {
    delete iface;
}




