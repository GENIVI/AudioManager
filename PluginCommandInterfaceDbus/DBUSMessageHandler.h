/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#ifndef _DBUSMESSAGEHANDLER_H_
#define _DBUSMESSAGEHANDLER_H_

#include <vector>
#include <sstream>
using std::stringstream;

#include <string>
using std::string;

#include "headers.h"
#include <dbus/dbus.h>

#define DLT_CONTEXT DBusCommandPlugin

typedef void (CommandDbusReceive::*CallBackMethod)(DBusConnection *connection, DBusMessage *message);

struct MethodTable
{
    const char *name;
    const char *signature;
    const char *reply;
    CallBackMethod function;
};

struct SignalTable {
	const char* name;
	const char* signature;
};


class DBUSMessageHandler
{
public:
	DBUSMessageHandler(DBusObjectPathVTable* vtable, DBusConnection* conn, void* reference);
    ~DBUSMessageHandler();

    void initReceive(DBusMessage* msg);
    void initReply(DBusMessage* msg);
    void closeReply();
    void ReplyError(DBusMessage* msg, const char* errorname, const char* errorMsg);

    dbus_uint32_t getUInt();
    char getByte();
    dbus_bool_t getBool();
    double getDouble();
    char* getString();
    void getArrayOfUInt(int* length, unsigned int** array);
    void getArrayOfString(std::vector<std::string>* uniforms);

    void append(dbus_uint32_t toAppend);
    void append(char toAppend);
    void append(bool toAppend);
    void append(double toAppend);
    void append(unsigned int length, unsigned int *IDs);
    void append(std::list<SourceType> list);
    void append(std::list<ConnectionType> list);
    void append(std::list<SinkType> list);

    void sendSignal(const char* signalname);

private:
    DBusMessageIter m_MessageIter;
    DBusMessage* m_pReply;
    dbus_uint32_t m_serial;
    DBusConnection* m_pConnection;
    DBusError m_err;
};

class DBUSIntrospection
{
public:
    DBUSIntrospection(MethodTable* table, SignalTable* stable,std::string nodename);
    void process(DBusConnection* conn, DBusMessage* msg);

private:
    void generateString(void);

    void addHeader(void);
    void addArgument(string argname, string direction, string type);
    void addSignalArgument(string argname, string type);
    void addEntry(MethodTable entry);
    void addEntry(SignalTable entry);

    void openNode(string nodename);
    void closeNode(void);

    void openInterface(string interfacename);
    void closeInterface(void);

    void openMethod(string methodname);
    void closeMethod(void);

    void openSignal(string signalname);
    void closeSignal(void);

private:
    stringstream m_introspectionString;
    MethodTable* m_methodTable;
    SignalTable* m_signalTable;
    std::string m_nodename;
};

#endif // _DBUSMESSAGEWRAPPER_H_
