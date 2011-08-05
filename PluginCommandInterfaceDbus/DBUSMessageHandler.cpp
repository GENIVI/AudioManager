

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
		DLT_LOG(DLT_CONTEXT, DLT_LOG_INFO, DLT_STRING("Registering of node"), DLT_STRING(MY_NODE),DLT_STRING("failed"));
	}
}

DBUSMessageHandler::~DBUSMessageHandler()
{
    DBusError err;
    dbus_error_init(&err);
    bool errorset = dbus_error_is_set(&err);
    if (errorset)
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("there was an dbus error"));
    }
    dbus_bus_name_has_owner(m_pConnection, DBUS_SERVICE_SERVICE, &err);
    errorset = dbus_error_is_set(&err);
    if (errorset)
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("there was an dbus error"));
    }
    dbus_error_init(&err);
    dbus_bus_release_name(m_pConnection, DBUS_SERVICE_SERVICE, &err);
}

void DBUSMessageHandler::initReceive(DBusMessage* msg)
{
    if (!dbus_message_iter_init(msg, &m_MessageIter))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS Message has no arguments!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler sending reply!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler sending reply with error!"));
    dbus_connection_flush(m_pConnection);

    // free the reply
    dbus_message_unref(m_pReply);
}

char* DBUSMessageHandler::getString()
{
    char* param;

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&m_MessageIter))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no string!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no bool!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no byte!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no uint32_t!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no double!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no array!"));
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no array!"));
        return;
    }

    DBusMessageIter arrayIter;
    dbus_message_iter_recurse(&m_MessageIter, &arrayIter);
    bool hasNext = true;
    while (hasNext)
    {
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&arrayIter))
        {
        	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler argument is no string!"));
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

void DBUSMessageHandler::append(bool toAppend)
{
	dbus_bool_t mybool=toAppend;
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_BOOLEAN, &mybool))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::append(dbus_uint32_t toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_UINT32, &toAppend))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::append(double toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_DOUBLE, &toAppend))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::append(char toAppend)
{
    if (!dbus_message_iter_append_basic(&m_MessageIter, DBUS_TYPE_BYTE, &toAppend))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
        exit(1);
    }
}

void DBUSMessageHandler::append(unsigned int length, unsigned int *IDs)
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

    string nodeString =std::string(DBUS_SERVICE_ROOT)+"/"+std::string(MY_NODE);
	msg =dbus_message_new_signal(nodeString.c_str(),DBUS_SERVICE_SERVICE,signalname);

	if (NULL == msg)
	{
		DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("Message null!"));
		this->~DBUSMessageHandler();
	}

	if (!dbus_connection_send(m_pConnection, msg, &serial)) {
		DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
		this->~DBUSMessageHandler();
	}

	dbus_connection_flush(m_pConnection);

	// free the message
	dbus_message_unref(msg);
}

void DBUSMessageHandler::append(std::list<ConnectionType> list){
    DBusMessageIter arrayIter;
    dbus_message_iter_open_container(&m_MessageIter, DBUS_TYPE_ARRAY, "uu", &arrayIter);

    std::list<ConnectionType>::iterator Ilist;
    std::list<ConnectionType>::iterator Ibegin=list.begin();
    std::list<ConnectionType>::iterator Iend=list.end();
    for(Ilist=Ibegin;Ilist!=Iend; Ilist++)
    {
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_UINT32, &Ilist->Sink_ID);
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_UINT32, &Ilist->Source_ID);
    }
    dbus_message_iter_close_container(&m_MessageIter, &arrayIter);
}

void DBUSMessageHandler::append(std::list<SinkType> list){
    DBusMessageIter arrayIter;
    dbus_message_iter_open_container(&m_MessageIter, DBUS_TYPE_ARRAY, "su", &arrayIter);

    std::list<SinkType>::iterator Ilist;
    std::list<SinkType>::iterator Ibegin=list.begin();
    std::list<SinkType>::iterator Iend=list.end();
    for(Ilist=Ibegin;Ilist!=Iend; Ilist++)
    {
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_STRING, &Ilist->name);
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_UINT32, &Ilist->ID);
    }
    dbus_message_iter_close_container(&m_MessageIter, &arrayIter);

}

void DBUSMessageHandler::append(std::list<SourceType> list){
    DBusMessageIter arrayIter;
    dbus_message_iter_open_container(&m_MessageIter, DBUS_TYPE_ARRAY, "su", &arrayIter);

    std::list<SourceType>::iterator Ilist;
    std::list<SourceType>::iterator Ibegin=list.begin();
    std::list<SourceType>::iterator Iend=list.end();
    for(Ilist=Ibegin;Ilist!=Iend; Ilist++)
    {
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_STRING, &Ilist->name);
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_UINT32, &Ilist->ID);
    }
    dbus_message_iter_close_container(&m_MessageIter, &arrayIter);
}



DBUSIntrospection::DBUSIntrospection(MethodTable* methodTable, SignalTable* signalTable,std::string nodename)
: m_methodTable(methodTable), m_signalTable(signalTable), m_nodename(nodename)
{
    generateString();
}

void DBUSIntrospection::generateString()
{
	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("Generating instrospection data!"));

    addHeader();
    openNode(m_nodename);
    openInterface("org.freedesktop.DBus.Introspectable");
    openMethod("Introspect");
    addArgument("data", "out", "s");
    closeMethod();
    closeInterface();
    openInterface(DBUS_SERVICE_SERVICE);

    int index = 0;

    while (strcmp(m_methodTable[index].name, "") != 0)
    {
        MethodTable entry = m_methodTable[index];
        addEntry(entry);
        ++index;
    }

    index=0;
    if (m_signalTable) {
        while (strcmp(m_signalTable[index].name, "") != 0)
        {
            SignalTable entry = m_signalTable[index];
            addEntry(entry);
            ++index;
        }
    }
    closeInterface();
    closeNode();

}

void DBUSIntrospection::addHeader(void)
{
    m_introspectionString << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS"
        << "Object Introspection 1.0//EN\"\n \"http://www.freedesktop.org/standards/"
        << "dbus/1.0/introspect.dtd\"> \n";
}

void DBUSIntrospection::openNode(string nodename)
{
    m_introspectionString << "<node name=\"" << nodename << "\">  \n";
}

void DBUSIntrospection::openInterface(string interfacename)
{
    m_introspectionString << "<interface name=\"" << interfacename << "\">  \n";
}

void DBUSIntrospection::openMethod(string methodname)
{
    m_introspectionString << "<method name=\"" << methodname << "\">  \n";
}

void DBUSIntrospection::openSignal(string signalname) {
	m_introspectionString<<"<signal name=\"" << signalname << "\">  \n";
}

void DBUSIntrospection::addArgument(string argname, string direction, string type)
{
    m_introspectionString << "<arg name=\"" << argname << "\" direction=\""
        << direction << "\" type=\"" << type << "\"/>  \n";
}


void DBUSIntrospection::addSignalArgument(string argname, string type){
    m_introspectionString << "<arg name=\"" << argname << "\" type=\"" << type << "\"/>  \n";
}

void DBUSIntrospection::closeMethod(void)
{
    m_introspectionString << "</method>  \n";
}

void DBUSIntrospection::closeInterface(void)
{
    m_introspectionString << "</interface>  \n";
}

void DBUSIntrospection::closeNode(void)
{
    m_introspectionString << "</node>  \n";
}

void DBUSIntrospection::closeSignal(void){
	m_introspectionString<<"</signal>  \n";
}

void DBUSIntrospection::addEntry(MethodTable entry)
{
    string methodName = entry.name;
    string parameterArray = entry.signature;
    string returnValueArray = string(entry.reply);

    openMethod(methodName);

    for(uint parameterIndex = 0; parameterIndex < parameterArray.length(); ++parameterIndex)
    {
        switch (parameterArray.at(parameterIndex))
        {
			case 'a':
			if (parameterArray.at(parameterIndex+1)=='(') {
				int size=parameterArray.find((')'),parameterIndex);
				addArgument("","in",parameterArray.substr(parameterIndex,size+1));
				parameterIndex+=size;
			} else {
				addArgument("","in", parameterArray.substr(parameterIndex,2));
				parameterIndex+=2;
			}
			break;
		default:
			addArgument("","in", parameterArray.substr(parameterIndex,1));
			break;
        }
    }


    for(uint returnValueIndex = 0; returnValueIndex < returnValueArray.length(); ++returnValueIndex)
    {
        switch (returnValueArray.at(returnValueIndex))
        {
			case 'a':
			if (returnValueArray.at(returnValueIndex+1)=='(') {
				int size=returnValueArray.find((')'),returnValueIndex);
				addArgument("","out",returnValueArray.substr(returnValueIndex,size+1));
				returnValueIndex+=size;
			} else {
				addArgument("","out", returnValueArray.substr(returnValueIndex,2));
				returnValueIndex+=2;
			}
			break;
		default:
			addArgument("","out", returnValueArray.substr(returnValueIndex,1));
			break;
        }
    }

    closeMethod();
}

void DBUSIntrospection::addEntry(SignalTable entry)
{
    string methodName = entry.name;
    string parameterArray = entry.signature;

    openSignal(methodName);

    for(uint parameterIndex = 0; parameterIndex < parameterArray.length(); ++parameterIndex)
    {
        switch (parameterArray.at(parameterIndex))
        {
            case 'a':
                if (parameterArray.at(parameterIndex+1)=='{') {
                	int size=parameterArray.find(('}'),parameterIndex);
            		addSignalArgument("",parameterArray.substr(parameterIndex,size+1));
            		parameterIndex+=size;
                } else {
                	parameterIndex++;
                    addSignalArgument("", "a" + parameterArray.at(parameterIndex));
                }
                break;
            default:
            	addSignalArgument("", parameterArray.substr(parameterIndex,1));
                break;
        }
    }

    closeSignal();
}

void DBUSIntrospection::process(DBusConnection* conn, DBusMessage* msg)
{
    DBusMessage * reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;

    // create a reply from the message
    reply = dbus_message_new_method_return(msg);

    string introspect = m_introspectionString.str();
    const char* string = introspect.c_str();

    // add the arguments to the reply
    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
    }

    // send the reply && flush the connection
    if (!dbus_connection_send(conn, reply, &serial))
    {
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
    }
    dbus_connection_flush(conn);

    // free the reply
    dbus_message_unref(reply);
}






