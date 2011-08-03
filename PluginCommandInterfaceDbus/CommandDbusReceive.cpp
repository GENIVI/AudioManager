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

DLT_IMPORT_CONTEXT(DBusCommandPlugin);

class CommandDbusReceive;


CommandDbusReceive* CommandDbusReceive::m_reference = NULL;

static DBUSMessageHandler* g_pDbusMessage;

static MethodTable manager_methods[] =
{
	{ "connect",        		"uu",    	"u",    	&CommandDbusReceive::connect },
	{ "disconnect",    			"uu",  		"u",    	&CommandDbusReceive::disconnect },
	{ "getListConnections",     "",    		"a{ii}",  	&CommandDbusReceive::getListConnections },
	{ "getListSinks",    		"",   		"a{si}",  	&CommandDbusReceive::getListSinks },
	{ "getListSources",      	"",    		"a{si}",  	&CommandDbusReceive::getListSources },
	{ "interruptRequest",      	"ss",    	"u",  		&CommandDbusReceive::interruptRequest },
	{ "interruptResume",    	"s",   		"u",  		&CommandDbusReceive::interruptResume },
	{ "setVolume",      		"ss",	    "u",  		&CommandDbusReceive::setVolume },
	{ "",                  "",      "",     NULL }
};

static SignalTable manager_signals[] = {
	{ "signal_connectionChanged",  	""},
	{ "signal_numberOfSinksChanged",  	""},
	{ "signal_numberOfSourcesChanged",  	""},
	{ "",                  		""}
};


CommandDbusReceive::CommandDbusReceive(CommandReceiveInterface* r_interface): m_audioman(r_interface),m_running(false) {
}

bool CommandDbusReceive::startup_interface() {
	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Starting up dbus connector"));

    g_pDbusMessage = new DBUSMessageHandler();
	DLT_LOG(DBusCommandPlugin,DLT_LOG_INFO, DLT_STRING("create thread"));
    this->m_running = true;
    pthread_create(&m_currentThread, NULL, CommandDbusReceive::run, this);
	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Started dbus connector"));
    return true;
}

void CommandDbusReceive::stop() {
	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Stopped dbus communication"));
    this->m_running = false;
    pthread_join(m_currentThread, NULL);
    delete g_pDbusMessage;
}

void CommandDbusReceive::connect(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	source_t source = g_pDbusMessage->getUInt();
	sink_t sink = g_pDbusMessage->getUInt();
	connection_t connect=m_audioman->connect(source, sink);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append((unsigned int)connect);
	g_pDbusMessage->closeReply();

}
void CommandDbusReceive::disconnect(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	source_t source = g_pDbusMessage->getUInt();
	sink_t sink = g_pDbusMessage->getUInt();
	connection_t connect=m_audioman->disconnect(source, sink);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append((unsigned int)connect);
	g_pDbusMessage->closeReply();
}

void CommandDbusReceive::getListConnections(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	std::list<ConnectionType> list=m_audioman->getListConnections();
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append(list);
	g_pDbusMessage->closeReply();

}

void CommandDbusReceive::getListSinks(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	std::list<SinkType> list=m_audioman->getListSinks();
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append(list);
	g_pDbusMessage->closeReply();

}

void CommandDbusReceive::getListSources(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	std::list<SourceType> list=m_audioman->getListSources();
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append(list);
	g_pDbusMessage->closeReply();

}

void CommandDbusReceive::interruptRequest(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	char* source = g_pDbusMessage->getString();
	char* sink = g_pDbusMessage->getString();
    genInt_t interrupt=m_audioman->interruptRequest(source, sink);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append((unsigned int)interrupt);
	g_pDbusMessage->closeReply();
}

void CommandDbusReceive::interruptResume(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	interrupt_t interrupt = g_pDbusMessage->getUInt();
	interrupt_t returnVal=m_audioman->interruptResume(interrupt);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append((unsigned int)returnVal);
	g_pDbusMessage->closeReply();
}

void CommandDbusReceive::setVolume(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	sink_t sink = g_pDbusMessage->getUInt();
	volume_t volume = g_pDbusMessage->getUInt();
	volume_t returnVolume=m_audioman->setVolume(sink,volume);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->append((unsigned int)returnVolume);
	g_pDbusMessage->closeReply();
}

void CommandDbusReceive::emitSignalConnectionsChanged() {
	g_pDbusMessage->sendSignal(SIG_CONNECTION_CHANGED);
}

void CommandDbusReceive::emitSignalNumberofSinksChanged() {
	g_pDbusMessage->sendSignal(SIG_NUM_SINKS_CHANGED);
}

void CommandDbusReceive::emitSignalNumberofSourcesChanged() {
	g_pDbusMessage->sendSignal(SIG_NUM_SOURCES_CHANGED);
}

DBusHandlerResult CommandDbusReceive::receive_callback (DBusConnection *conn,DBusMessage *msg,void *user_data) {
	(void) user_data;
	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("message received"));

	const char *member = dbus_message_get_member(msg);
	const char *iface = dbus_message_get_interface(msg);
	bool found=false;
	int i = 0;

	if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
		DBUSIntrospection introspectionString(manager_methods,manager_signals);
		introspectionString.process(conn, msg); 'a'
		g_pDbusMessage->setConnection(conn);
		dbus_connection_pop_message(conn);
		return DBUS_HANDLER_RESULT_HANDLED;
	} else if (strcmp(DBUS_SERVICE_PREFIX,iface)==0) {

		while (!found && strcmp(manager_methods[i].name, "") != 0) {
			if (strcmp(manager_methods[i].name, member) == 0)
			{
				MethodTable entry = manager_methods[i];
				DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("got call for method:"),DLT_STRING(entry.name));
				CallBackMethod m = entry.function;
				(m_reference->*m)(conn, msg);
				found = true;
			}
			i++;
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	} else {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
}

void* CommandDbusReceive::run(void * arg)
{
	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Main loop running"));
    m_reference = (CommandDbusReceive*) arg;
    DBusConnection* conn = g_pDbusMessage->getConnection();
    if (!dbus_connection_add_filter(conn, (DBusHandleMessageFunction)&m_reference->receive_callback, NULL,NULL)) {
    	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("failing filter thread"));
    }
    while (m_reference->m_running && dbus_connection_read_write_dispatch(conn, -1))
    {
    	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Dispatching Most message..."));
    }

	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Stopping thread"));
}

