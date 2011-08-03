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

class AudioManagerInterface;


AudioManagerInterface* AudioManagerInterface::m_reference = NULL;

static DBUSMessageHandler* g_pDbusMessage;

static MethodTable manager_methods[] =
{
	{ "peekDomain",        "s",    "u",     &AudioManagerInterface::peekDomain },
	{ "registerSource",    "sss",  "u",     &AudioManagerInterface::registerSource },
	{ "registerSink",      "sss",    "u",  	&AudioManagerInterface::registerSink },
	{ "registerDomain",    "sssb",   "u",  	&AudioManagerInterface::registerDomain },
	{ "registerGateway",      "sssss",    "u",  	&AudioManagerInterface::registerGateway },
	{ "",                  "",      "",     NULL }
};

static SignalTable manager_signals[] = {
	{ "signal_systemReady",  	""},
	{ "",                  		""}
};

static DBusObjectPathVTable _vtable =
{
	NULL,AudioManagerInterface::receive_callback,NULL, NULL, NULL, NULL
};



AudioManagerInterface::AudioManagerInterface(RoutingReceiveInterface* r_interface, dbusRoothandler* roothandler) : m_audioman(r_interface),m_Introspection(new DBUSIntrospection(manager_methods, manager_signals,std::string(MY_NODE))),m_roothandler(roothandler) {
}

bool AudioManagerInterface::startup_interface()
{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Starting up dbus connector"));

    g_pDbusMessage = new DBUSMessageHandler(&_vtable,m_roothandler->returnConnection(),this);

    return true;
}

void AudioManagerInterface::stop()

{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Stopped dbus connector"));
    delete g_pDbusMessage;
}

void AudioManagerInterface::peekDomain(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	char* name = g_pDbusMessage->getString();
	domain_t domain = m_audioman->peekDomain(name);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(domain);
	g_pDbusMessage->closeReply();
}

void AudioManagerInterface::registerSource(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	char* name = g_pDbusMessage->getString();
	char* audioclass = g_pDbusMessage->getString();
	char* domain = g_pDbusMessage->getString();
	source_t source=m_audioman->registerSource(name, audioclass, domain);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(source);
	g_pDbusMessage->closeReply();
}
void AudioManagerInterface::registerSink(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	char* name = g_pDbusMessage->getString();
	char* audioclass = g_pDbusMessage->getString();
	char* domain = g_pDbusMessage->getString();
	sink_t sink=m_audioman->registerSink(name, audioclass, domain);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(sink);
	g_pDbusMessage->closeReply();
}

void AudioManagerInterface::registerDomain(DBusConnection* conn, DBusMessage* msg) {
	char busname[40];
	strcpy(busname, BUS_NAME);
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	char* name = g_pDbusMessage->getString();
	char* node = g_pDbusMessage->getString();
	bool earlymode = g_pDbusMessage->getString();
	domain_t domain=m_reference->m_audioman->registerDomain(name, busname, node, earlymode);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(domain);
	g_pDbusMessage->closeReply();

}
void AudioManagerInterface::registerGateway(DBusConnection* conn, DBusMessage* msg) {
	(void)conn; // TODO: remove, only prevents warning
	g_pDbusMessage->initReceive(msg);
	char* name = g_pDbusMessage->getString();
	char* sink = g_pDbusMessage->getString();
	char* source = g_pDbusMessage->getString();
	char* domainSource = g_pDbusMessage->getString();
	char* domainSink = g_pDbusMessage->getString();
	char* controlDomain = g_pDbusMessage->getString();
	domain_t domain=m_audioman->registerGateway(name, sink, source, domainSource, domainSink, controlDomain);
	g_pDbusMessage->initReply(msg);
	g_pDbusMessage->appendUInt(domain);
	g_pDbusMessage->closeReply();
	emit_systemReady();
}
void AudioManagerInterface::emit_systemReady() {
	g_pDbusMessage->sendSignal("signal_systemReady");
}

DBusHandlerResult AudioManagerInterface::receive_callback(DBusConnection *conn, DBusMessage *msg, void *user_data){
	m_reference=(AudioManagerInterface*) user_data;
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("message received"));

    string nodeString =std::string(DBUS_SERVICE_ROOT)+"/"+std::string(MY_NODE);

	if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
		m_reference->m_Introspection->process(conn,msg);
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	bool found=false;
	int i = 0;

	while (!found && strcmp(manager_methods[i].name, "") != 0) {
		if (dbus_message_is_method_call(msg,DBUS_SERVICE_SERVICE,manager_methods[i].name)) {
			MethodTable entry = manager_methods[i];
			DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("got call for method:"),DLT_STRING(entry.name));
			CallBackMethod m = entry.function;
			(m_reference->*m)(conn, msg);
			found=true;
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		i++;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}



