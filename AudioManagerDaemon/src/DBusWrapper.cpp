/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file DBusWrapper.cpp
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
*/

#include "DBusWrapper.h"
#include <dlt/dlt.h>
#include <fstream>
#include <sstream>
#include <string>

DLT_IMPORT_CONTEXT(AudioManager)


#define ROOT_INTROSPECT_XML												\
DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE								\
"<node>"																\
"<interface name='org.freedesktop.DBus.Introspectable'>"				\
"<method name='Introspect'>"											\
"	<arg name='xml_data' type='s' direction='out'/>"					\
"</method>"							      								\
"</interface>"															\

DBusWrapper* DBusWrapper::mReference = NULL;


DBusWrapper::DBusWrapper()
	: mDbusConnection(0),
	  mDBusError(),
	  mNodesList()
{
    dbus_error_init(&mDBusError);

    DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBusWrapper::DBusWrapper Opening DBus connection"));

    mDbusConnection=dbus_bus_get(DBUS_BUS_SESSION, &mDBusError);
    if (dbus_error_is_set(&mDBusError))
    {
    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::DBusWrapper Error while getting the DBus"));
        dbus_error_free(&mDBusError);
    }
    if (NULL == mDbusConnection)
    {
    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::DBusWrapper DBus Connection is null"));
    }

	mObjectPathVTable.message_function=DBusWrapper::cbRootIntrospection;
	dbus_connection_register_object_path(mDbusConnection,DBUS_SERVICE_OBJECT_PATH , &mObjectPathVTable, this);
	int ret = dbus_bus_request_name(mDbusConnection,DBUS_SERVICE_PREFIX,DBUS_NAME_FLAG_DO_NOT_QUEUE, &mDBusError);
    if (dbus_error_is_set(&mDBusError))
    {
    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::DBusWrapper Name Error"),DLT_STRING(mDBusError.message));
        dbus_error_free(&mDBusError);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
    {
    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::DBusWrapper Wrapper is not the Primary Owner"), DLT_INT(ret));
    }
}

DBusWrapper::~DBusWrapper()
{
	//close the connection again
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBusWrapper::~DBusWrapper Closing DBus connection"));
	dbus_connection_unref(mDbusConnection);
}

void DBusWrapper::registerCallback(const DBusObjectPathVTable* vtable, const std::string& path, void* userdata)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBusWrapper::~registerCallback register callback:"),DLT_STRING(path.c_str()));

	std::string completePath=std::string(DBUS_SERVICE_OBJECT_PATH)+"/"+path;
	dbus_error_init(&mDBusError);
	mDbusConnection=dbus_bus_get(DBUS_BUS_SESSION, &mDBusError);
	dbus_connection_register_object_path(mDbusConnection,completePath.c_str(), vtable, userdata);
    if (dbus_error_is_set(&mDBusError))
    {
    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::registerCallack error: "),DLT_STRING(mDBusError.message));
        dbus_error_free(&mDBusError);
    }
	mNodesList.push_back(path);
}

DBusHandlerResult DBusWrapper::cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBusWrapper::~cbRootIntrospection called:"));

	mReference=(DBusWrapper*)reference;
	std::list<std::string>nodesList=mReference->mNodesList;
	DBusMessage * reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;
	if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{
		std::list<std::string>::iterator nodeIter=nodesList.begin();
		const char *xml=ROOT_INTROSPECT_XML;
		std::stringstream introspect;
		introspect << std::string(xml);
		for(;nodeIter!=nodesList.end();++nodeIter)
		{
			introspect<<"<node name='"<<nodeIter->c_str()<<"'/>";
		}
		introspect<<"</node>";

		reply = dbus_message_new_method_return(msg);
	    std::string s = introspect.str();
	    const char* string=s.c_str();

	    // add the arguments to the reply
	    dbus_message_iter_init_append(reply, &args);
	    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string))
	    {
	    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::~cbRootIntrospection DBUS Out Of Memory!"));
	    }

	    // send the reply && flush the connection
	    if (!dbus_connection_send(conn, reply, &serial))
	    {
	    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBusWrapper::~cbRootIntrospection DBUS Out Of Memory!"));
	    }
	    dbus_connection_flush(conn);
	    // free the reply
	    dbus_message_unref(reply);

		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else
	{
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
}

void DBusWrapper::dbusMainLoop()
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBusWrapper::~dbusMainLoop Entering MainLoop"));

	while (dbus_connection_read_write_dispatch(mDbusConnection, -1))
	{

	}
}

void DBusWrapper::getDBusConnection(DBusConnection *& connection) const
{
	connection=mDbusConnection;
}
















