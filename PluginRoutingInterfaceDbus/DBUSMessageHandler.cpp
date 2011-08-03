

#include "headers.h"
#include <stdlib.h>

DBUSMessageHandler::DBUSMessageHandler(DBusObjectPathVTable* vtable, DBusConnection* conn, void* reference)
: m_MessageIter()
, m_pReply(0)
, m_serial(0)
, m_pConnection(conn)
{
    dbus_error_init(&m_err);

    string nodeString =std::string(DBUS_SERVICE_ROOT)+"/"+std::string(MY_NODE);
	dbus_bool_t b=dbus_connection_register_object_path(m_pConnection, nodeString.c_str(), vtable, reference);
	if(!b) {
		DLT_LOG(DBusPlugin, DLT_LOG_INFO, DLT_STRING("Registering of node"), DLT_STRING(MY_NODE),DLT_STRING("failed"));
	}
}

DBUSMessageHandler::~DBUSMessageHandler()
{
    DBusError err;
    dbus_error_init(&err);
    bool errorset = dbus_error_is_set(&err);
    if (errorset)
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("there was an dbus error"));
    }
    dbus_bus_name_has_owner(m_pConnection, DBUS_SERVICE_SERVICE, &err);
    errorset = dbus_error_is_set(&err);
    if (errorset)
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("there was an dbus error"));
    }
    dbus_error_init(&err);
    dbus_bus_release_name(m_pConnection, DBUS_SERVICE_SERVICE, &err);
}

void DBUSMessageHandler::initReceive(DBusMessage* msg)
{
    if (!dbus_message_iter_init(msg, &m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS Message has no arguments!"));
    }
}

void DBUSMessageHandler::initReply(DBusMessage* msg)
{
    // create a reply from the message
    m_pReply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(m_pReply, &m_MessageIter);
}

void DBUSMessageHandler::closeReply()
{
    // send the reply && flush the connection
    if (!dbus_connection_send(m_pConnection, m_pReply, &m_serial))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler sending reply!"));
    dbus_connection_flush(m_pConnection);

    // free the reply
    dbus_message_unref(m_pReply);
    m_pReply = NULL;
}

void DBUSMessageHandler::ReplyError(DBusMessage* msg, const char* errorname, const char* errorMsg)
{
    m_pReply = dbus_message_new_error(msg, errorname, errorMsg);
    // send the reply && flush the connection
    if (!dbus_connection_send(m_pConnection, m_pReply, &m_serial))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler sending reply with error!"));
    dbus_connection_flush(m_pConnection);

    // free the reply
    dbus_message_unref(m_pReply);
}

char* DBUSMessageHandler::getString()
{
    char* param;

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no string!"));
    }
    else
    {
        dbus_message_iter_get_basic(&m_MessageIter, &param);
        dbus_message_iter_next(&m_MessageIter);
    }
    return param;
}

dbus_bool_t DBUSMessageHandler::getBool()
{
    dbus_bool_t boolparam;

    if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no bool!"));
    }
    else
    {
        dbus_message_iter_get_basic(&m_MessageIter, &boolparam);
        dbus_message_iter_next(&m_MessageIter);
    }
    return boolparam;
}

char DBUSMessageHandler::getByte()
{
    char param;

    if (DBUS_TYPE_BYTE != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no byte!"));
    }
    else
    {
        dbus_message_iter_get_basic(&m_MessageIter, &param);
        dbus_message_iter_next(&m_MessageIter);
    }
    return param;
}

dbus_uint32_t DBUSMessageHandler::getUInt()
{
    dbus_uint32_t param;

    if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no uint32_t!"));
    }
    else
    {
        dbus_message_iter_get_basic(&m_MessageIter, &param);
        dbus_message_iter_next(&m_MessageIter);
    }
    return param;
}

double DBUSMessageHandler::getDouble()
{
    double param;

    if (DBUS_TYPE_DOUBLE != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no double!"));
    }
    else
    {
        dbus_message_iter_get_basic(&m_MessageIter, &param);
        dbus_message_iter_next(&m_MessageIter);
    }
    return param;
}

void DBUSMessageHandler::getArrayOfUInt(int* pLength, unsigned int** ppArray)
{
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no array!"));
        return;
    }

    DBusMessageIter arrayIter;
    dbus_message_iter_recurse(&m_MessageIter, &arrayIter);

    uint* localArray;
    dbus_message_iter_get_fixed_array(&arrayIter, &localArray, pLength);

    *ppArray = new uint[*pLength];
    for (int i = 0; i < *pLength; i++)
    {
        (*ppArray)[i] = localArray[i];
    }
}

void DBUSMessageHandler::getArrayOfString(std::vector<std::string>* stringVector)
{
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no array!"));
        return;
    }

    DBusMessageIter arrayIter;
    dbus_message_iter_recurse(&m_MessageIter, &arrayIter);
    bool hasNext = true;
    while (hasNext)
    {
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&arrayIter))
        {
        	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no string!"));
        }
        char* param;
        dbus_message_iter_get_basic(&arrayIter, &param);

        stringVector->push_back(std::string(param));

        if (dbus_message_iter_has_next(&arrayIter))
        {
            dbus_message_iter_next(&arrayIter);
        }
        else
        {
            hasNext = false;
        }
    }
}

void DBUSMessageHandler::appendBool(dbus_bool_t toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_BOOLEAN, &toAppend))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::appendUInt(dbus_uint32_t toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_UINT32, &toAppend))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::appendDouble(double toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_DOUBLE, &toAppend))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::appendByte(char toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_BYTE, &toAppend))
    {
    	DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::appendArrayOfUInt(unsigned int length, unsigned int *IDs)
{
    DBusMessageIter arrayIter;
    dbus_message_iter_open_container(&m_MessageIter, DBUS_TYPE_ARRAY, "u", &arrayIter);
    for(unsigned int i = 0; i < length; i++)
    {
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_UINT32, &IDs[i]);
    }
    dbus_message_iter_close_container(&m_MessageIter, &arrayIter);
}

void DBUSMessageHandler::sendSignal(const char* signalname) {
	dbus_uint32_t serial = 0;
	DBusMessage* msg;

	msg =dbus_message_new_signal(DBUS_SERVICE_ROOT,DBUS_SERVICE_SERVICE,signalname);

	if (NULL == msg)
	{
		DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("Message null!"));
		this->~DBUSMessageHandler();
	}

	if (!dbus_connection_send(m_pConnection, msg, &serial)) {
		DLT_LOG(DBusPlugin,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
		this->~DBUSMessageHandler();
	}

	dbus_connection_flush(m_pConnection);

	// free the message
	dbus_message_unref(msg);
}
