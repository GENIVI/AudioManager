/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * PluginDBus
 *
 * \file DBusInterface.cpp
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
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

class CommandReceive;


CommandReceive* CommandReceive::m_reference = NULL;

static DBUSMessageHandler* g_pDbusMessage;

static MethodTable manager_methods[] =
{
	{ "connect",        		"uu",    	"u",    	&CommandReceive::connect },
	{ "disconnect",    			"uu",  		"u",    	&CommandReceive::disconnect },
	{ "getListConnections",     "",    		"a{ii}",  	&CommandReceive::getListConnections },
	{ "getListSinks",    		"",   		"a{si}",  	&CommandReceive::getListSinks },
	{ "getListSources",      	"",    		"a{si}",  	&CommandReceive::getListSources },
	{ "interruptRequest",      	"ss",    	"u",  		&CommandReceive::interruptRequest },
	{ "interruptResume",    	"s",   		"u",  		&CommandReceive::interruptResume },
	{ "setVolume",      		"ss",	    "u",  		&CommandReceive::setVolume },
	{ "",                  "",      "",     NULL }
};

static SignalTable manager_signals[] = {
	{ "signal_connectionChanged",  	""},
	{ "signal_numberOfSinksChanged",  	""},
	{ "signal_numberOfSourcesChanged",  	""},
	{ "",                  		""}
};


CommandReceive::CommandReceive(RoutingReceiveInterface* r_interface) : m_audioman(r_interface),m_running(false) {
}

bool CommandReceive::startup_interface()
{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Starting up dbus connector"));

    g_pDbusMessage = new DBUSMessageHandler();
	DLT_LOG(DBusPlugin,DLT_LOG_INFO, DLT_STRING("create thread"));
    this->m_running = true;
    pthread_create(&m_currentThread, NULL, CommandReceive::run, this);
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Started dbus connector"));
    return true;
}

void CommandReceive::stop()

{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Stopped dbus connector"));
    this->m_running = false;
    pthread_join(m_currentThread, NULL);
    delete g_pDbusMessage;
}

void CommandReceive::connect(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	source_t source = g_pDbusMessage->getUInt();
	sink_t sink = g_pDbusMessage->getUInt();
	connection_t connect=m_audioman->connect(source, sink);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(connect);
	g_pDbusMessage->closeReply();

}
void CommandReceive::disconnect(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	source_t source = g_pDbusMessage->getUInt();
	sink_t sink = g_pDbusMessage->getUInt();
	connection_t connect=m_audioman->disconnect(source, sink);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(connect);
	g_pDbusMessage->closeReply();
}

void CommandReceive::getListConnections(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	std::list<ConnectionType> list=m_audioman->getListConnections();
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendArrayOfStringString()
	g_pDbusMessage->closeReply();

}

void CommandReceive::getListSinks(DBusConnection* conn, DBusMessage* msg) {

}

void CommandReceive::getListSources(DBusConnection* conn, DBusMessage* msg) {

}

void CommandReceive::interruptRequest(DBusConnection* conn, DBusMessage* msg) {

}

void CommandReceive::interruptResume(DBusConnection* conn, DBusMessage* msg) {

}

void CommandReceive::setVolume(DBusConnection* conn, DBusMessage* msg) {

}

void* CommandReceive::run(void * arg)
{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Main loop running"));
    m_reference = (CommandReceive*) arg;
    DBusMessage* msg = 0;
    DBusConnection* conn = g_pDbusMessage->getConnection();
    while (m_reference->m_running && dbus_connection_read_write_dispatch(conn, -1))
    {
    	msg = dbus_connection_pop_message(conn);
		DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("message received"));

		const char *member = dbus_message_get_member(msg);
		const char *iface = dbus_message_get_interface(msg);
		bool found=false;
        int i = 0;

		if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
			DBUSIntrospection introspectionString(manager_methods,manager_signals);
			introspectionString.process(conn, msg);
			g_pDbusMessage->setConnection(conn);
		} else if (strcmp(DBUS_SERVICE_PREFIX,iface)==0) {

			while (!found && strcmp(manager_methods[i].name, "") != 0)
			{
				if (strcmp(manager_methods[i].name, member) == 0)
				{
					MethodTable entry = manager_methods[i];
					DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("got call for method:"),DLT_STRING(entry.name));
					CallBackMethod m = entry.function;
					(m_reference->*m)(conn, msg);
					found = true;
				}
				i++;
			}
		}
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		msg = NULL;
    }

	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Stopping thread"));
    return 0;
}

