/*
 * dbusRoothandler.cpp
 *
 *  Created on: Aug 1, 2011
 *      Author: christian
 */

#include "audioManagerIncludes.h"
#ifdef WITH_DBUS

#include "dbusRoothandler.h"

#define ROOT_INTROSPECT_XML												\
DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE								\
"<node>"																\
"<interface name='org.freedesktop.DBus.Introspectable'>"				\
"<method name='Introspect'>"											\
"	<arg name='xml_data' type='s' direction='out'/>"					\
"</method>"							      								\
"</interface>"															\


dbusRoothandler* dbusRoothandler::m_reference = NULL;

static DBusObjectPathVTable vtable =
{
	NULL,dbusRoothandler::cbRootIntrospection,NULL, NULL, NULL, NULL
};


DBusHandlerResult dbusRoothandler::cbRootIntrospection(DBusConnection *conn, DBusMessage *msg, void *reference) {
	m_reference=(dbusRoothandler*)reference;
	std::list<std::string>nodesList=m_reference->m_nodesList;
	DBusMessage * reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;
	DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("Root Callback called"));
	if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
		std::list<std::string>::iterator nodeIter;
		std::list<std::string>::iterator nodeIterStart=nodesList.begin();
		std::list<std::string>::iterator nodeIterEnd=nodesList.end();
		const char *xml=ROOT_INTROSPECT_XML;
		std::stringstream introspect;
		introspect << std::string(xml);
		for(nodeIter=nodeIterStart;nodeIter!=nodeIterEnd;nodeIter++) {
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
	    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBUS Out Of Memory!"));
	    }

	    // send the reply && flush the connection
	    if (!dbus_connection_send(conn, reply, &serial))
	    {
	    	DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("DBUS Out Of Memory!"));
	    }
	    dbus_connection_flush(conn);
	    // free the reply
	    dbus_message_unref(reply);

		return DBUS_HANDLER_RESULT_HANDLED;
	} else {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
}

dbusRoothandler::dbusRoothandler() : m_pConnection(0),m_err(),m_nodesList() {
    dbus_error_init(&m_err);

    // connect to the bus and check for errors
    m_pConnection = dbus_bus_get(DBUS_BUS_SESSION, &m_err);
    if (dbus_error_is_set(&m_err))
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Error while getting the DBus"));
        dbus_error_free(&m_err);
    }
    if (NULL == m_pConnection)
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBus Connection is null"));
    }

    dbus_connection_register_object_path(m_pConnection, DBUS_SERVICE_ROOT, &vtable, this);
    int ret = dbus_bus_request_name(m_pConnection,DBUS_SERVICE_SERVICE,DBUS_NAME_FLAG_DO_NOT_QUEUE, &m_err);
    if (dbus_error_is_set(&m_err))
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBUSCommunicator Name Error"),DLT_STRING(m_err.message));
        dbus_error_free(&m_err);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("DBUSCommunicatorNot Primary Owner"), DLT_INT(ret));
    }

	// TODO Auto-generated constructor stub

}

dbusRoothandler::~dbusRoothandler() {
	// TODO Auto-generated destructor stub
}


DBusConnection *dbusRoothandler::returnConnection(){
	return m_pConnection;
}

void dbusRoothandler::registerNode(std::string node){
	m_nodesList.push_back(node);
}

#endif




