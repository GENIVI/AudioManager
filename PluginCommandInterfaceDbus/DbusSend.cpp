/*
 * DbusSend.cpp
 *
 *  Created on: Jul 20, 2011
 *      Author: christian
 */

#include "DbusSend.h"

DbusSend::DbusSend (DBusConnection* conn, const char* bus_name,const char* path, const char* interface, const char* method) : m_conn(conn) {
	m_msg=dbus_message_new_method_call(bus_name,path,interface,method);
	if (NULL == m_msg) {
		  DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Error while creating DBus message"));
	      this->~DbusSend();
	}
}

DbusSend::~DbusSend() {
	// TODO Auto-generated destructor stub
}

void DbusSend::appendString(char* string) {
	dbus_message_iter_init_append(m_msg, &m_args);
	if (!dbus_message_iter_append_basic(&m_args, DBUS_TYPE_STRING, string)) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Out of memory"));
	 	this->~DbusSend();
	}
}

void DbusSend::appendInteger(int integer) {
	dbus_message_iter_init_append(m_msg, &m_args);
	if (!dbus_message_iter_append_basic(&m_args, DBUS_TYPE_INT32, &integer)) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Out of memory"));
	 	this->~DbusSend();
	}

}

void DbusSend::sendReply(bool* reply) {

	DBusPendingCall* pending;
	DBusMessageIter args;
	if (!dbus_connection_send_with_reply (m_conn, m_msg, &pending, -1)) { // -1 is default timeout
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Out of memory"));
		this->~DbusSend();
	}

	if (NULL == pending) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Pending Call Null"));
		this->~DbusSend();
	}
	dbus_connection_flush(m_conn);
	dbus_message_unref(m_msg);
    dbus_pending_call_block(pending);

    DBusMessage* msg=dbus_pending_call_steal_reply(pending);

	if (NULL == msg) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("DBUS reply NULL"));
		this->~DbusSend();
	}

	dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args)) {
    	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("DBUS Message without arguments"));
    } else if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args)) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Argument not boolean"));
	} else {
		dbus_message_iter_get_basic(&args, reply);
	}

    dbus_message_unref(msg);
}

void DbusSend::sendReply(int* reply) {

	DBusPendingCall* pending;
	DBusMessageIter args;
	if (!dbus_connection_send_with_reply (m_conn, m_msg, &pending, -1)) { // -1 is default timeout
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Out of memory"));
		this->~DbusSend();
	}

	if (NULL == pending) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Pending Call Null"));
		this->~DbusSend();
	}
	dbus_connection_flush(m_conn);
	dbus_message_unref(m_msg);
    dbus_pending_call_block(pending);

    DBusMessage* msg=dbus_pending_call_steal_reply(pending);

	if (NULL == msg) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("DBUS reply NULL"));
		this->~DbusSend();
	}

	dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args)) {
    	DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("DBUS Message without arguments"));
    } else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
		DLT_LOG(DBusCommandPlugin, DLT_LOG_INFO, DLT_STRING("Argument not integer"));
	} else {
		dbus_message_iter_get_basic(&args, reply);
	}

    dbus_message_unref(msg);
}

