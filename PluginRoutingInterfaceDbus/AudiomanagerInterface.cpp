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

class AudioManagerInterface;

AudioManagerInterface* AudioManagerInterface::m_reference = NULL;

static DBUSMessageHandler* g_pDbusMessage;

static MethodTable manager_methods[] =
{
	{ "registerSource",                   "sss",  "u",           &AudioManagerInterface::registerSource },
    { "peekDomain",                       "s",    "u",           &AudioManagerInterface::peekDomain },
    { "",                                 "",      "",            NULL }
};

AudioManagerInterface::AudioManagerInterface(RoutingReceiveInterface* r_interface) : m_audioman(r_interface),m_running(false) {
}

bool AudioManagerInterface::startup_interface()
{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Starting up dbus connector"));

    g_pDbusMessage = new DBUSMessageHandler();

	DLT_LOG(DBusPlugin,DLT_LOG_INFO, DLT_STRING("create thread"));
    this->m_running = true;
    pthread_create(&m_currentThread, NULL, AudioManagerInterface::run, this);
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Started dbus connector"));
    return true;
}

void AudioManagerInterface::stop()
{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Stopped dbus connector"));
    this->m_running = false;
    pthread_join(m_currentThread, NULL);
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
void AudioManagerInterface::registerSink(const char* &name, const char* &sinkclass, const char* &domain) {
	 m_audioman->registerSink((char*)name, (char*)sinkclass, (char*)domain);
}
void AudioManagerInterface::registerDomain(const char* &name, const char* &node,	bool earlymode) {
	char busname[40];
	strcpy(busname, BUS_NAME);
	 m_audioman->registerDomain((char*)name, busname, (char*)node, earlymode);
}
void AudioManagerInterface::registerGateway(const char* &name, const char* &sink, const char* &source, const char* &domainSource, const char* &domainSink, const char* &controlDomain) {
	 m_audioman->registerGateway((char*)name, (char*)sink, (char*)source, (char*)domainSource, (char*)domainSink, (char*)controlDomain);
}
void AudioManagerInterface::emit_systemReady() {
	m_reference->emit_systemReady();
}

void* AudioManagerInterface::run(void * arg)
{
	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Main loop running"));
    //    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    //    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    m_reference = (AudioManagerInterface*) arg;
    while (m_reference->m_running)
    {
        //        pthread_testcancel();
        DBusMessage* msg = 0;
        DBusConnection* conn = g_pDbusMessage->getConnection();
        dbus_connection_read_write(conn, 50);
        msg = dbus_connection_pop_message(conn);
        if (msg)
        {
        	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("message received"));
            const char *n = dbus_message_get_member(msg);
            bool found = false;
            int i = 0;

            while (!found && strcmp(manager_methods[i].name, "") != 0)
            {
                if (n && strcmp(manager_methods[i].name, n) == 0)
                {
                    MethodTable entry = manager_methods[i];
                	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("got call for method:"),DLT_STRING(entry.name));
                    CallBackMethod m = entry.function;
                    (m_reference->*m)(conn, msg);
                    found = true;
                }
                i++;
            }

            if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
            {

                DBUSIntrospection introspectionString(manager_methods);
                introspectionString.process(conn, msg);
                g_pDbusMessage->setConnection(conn);
                found = true; // TODO: always true
            }

            if (!found)
            {
                DBusMessage* reply = dbus_message_new_method_return(msg);
                uint serial = 0;
                // send the reply && flush the connection
                if (!dbus_connection_send(conn, reply, &serial))
                {
                	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Out of memory"));
                }
                dbus_connection_flush(conn);
                // free the reply
                dbus_message_unref(reply);
                reply = NULL;
            }
            if (msg)
            {
                dbus_connection_flush(conn);
                dbus_message_unref(msg);
                msg = NULL;
            }
        } else {
            /* put thread in sleep mode for 500 useconds due to safe cpu performance */
            //usleep(500);
        }
    }

	DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Stopping thread"));
    return 0;
}

