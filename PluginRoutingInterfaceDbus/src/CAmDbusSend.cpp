/**
 *  Copyright (c) copyright 2011-2012 Aricent® Group  and its licensors
 *
 *  \author: Sampreeth Ramavana
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "CAmDbusSend.h"
#include <dlt/dlt.h>

DLT_IMPORT_CONTEXT(DLT_CONTEXT)

CAmDbusSend::CAmDbusSend (DBusConnection* conn, const char* bus_name,const char* path, const char* interface, const char* method) : m_conn(conn) {
	m_msg=dbus_message_new_method_call(bus_name,path,interface,method);
	if (NULL == m_msg) {
              this->~CAmDbusSend();
	}
}

CAmDbusSend::~CAmDbusSend() {
        if(replymsg)
        dbus_message_unref(replymsg);
}

void CAmDbusSend::appendString(char* string) {
	dbus_message_iter_init_append(m_msg, &m_args);
	if (!dbus_message_iter_append_basic(&m_args, DBUS_TYPE_STRING, string)) {
                this->~CAmDbusSend();
	}
}

void CAmDbusSend::appendInteger(int integer) {
	dbus_message_iter_init_append(m_msg, &m_args);
	if (!dbus_message_iter_append_basic(&m_args, DBUS_TYPE_INT32, &integer)) {
                this->~CAmDbusSend();
	}

}

void CAmDbusSend::sendReply(bool* reply) {

	DBusPendingCall* pending;
	DBusMessageIter args;
	if (!dbus_connection_send_with_reply (m_conn, m_msg, &pending, -1)) { // -1 is default timeout
                this->~CAmDbusSend();
	}

	if (NULL == pending) {
                this->~CAmDbusSend();
	}
	dbus_connection_flush(m_conn);
	dbus_message_unref(m_msg);
    dbus_pending_call_block(pending);

    DBusMessage* msg=dbus_pending_call_steal_reply(pending);

	if (NULL == msg) {
                this->~CAmDbusSend();
	}

	dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args)) {
    } else if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args)) {
	} else {
		dbus_message_iter_get_basic(&args, reply);
	}

    dbus_message_unref(msg);
}

void CAmDbusSend::sendReply(int* reply) {

	DBusPendingCall* pending;
	DBusMessageIter args;
	if (!dbus_connection_send_with_reply (m_conn, m_msg, &pending, -1)) { // -1 is default timeout
                this->~CAmDbusSend();
	}

	if (NULL == pending) {
                this->~CAmDbusSend();
	}
	dbus_connection_flush(m_conn);
	dbus_message_unref(m_msg);
    dbus_pending_call_block(pending);

    DBusMessage* msg=dbus_pending_call_steal_reply(pending);

	if (NULL == msg) {
                this->~CAmDbusSend();
	}

	dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args)) {
    } else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
	} else {
		dbus_message_iter_get_basic(&args, reply);
	}

    dbus_message_unref(msg);
}

void CAmDbusSend::sendReply(void) {

        if (!dbus_connection_send(m_conn, m_msg, NULL))
        {
                this->~CAmDbusSend();
        }
        replymsg = NULL;

}
void CAmDbusSend::Replyint32(int *reply)
{
    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
    }
    else
    {
                dbus_message_iter_get_basic(&mDBusMessageIter, reply);
    }
    dbus_message_iter_next(&mDBusMessageIter);
}
