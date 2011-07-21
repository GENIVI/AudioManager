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
#include <string>
#include "headers.h"

class DBUSMessageHandler
{
public:
    DBUSMessageHandler();
    ~DBUSMessageHandler();

    void setConnection(DBusConnection* conn);
    DBusConnection* getConnection();

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

    void appendUInt(dbus_uint32_t toAppend);
    void appendByte(char toAppend);
    void appendBool(dbus_bool_t toAppend);
    void appendDouble(double toAppend);
    void appendArrayOfUInt(unsigned int length, unsigned int *IDs);

    void sendSignal(const char* name,const char* signal);

private:
    DBusMessageIter m_MessageIter;
    DBusMessage* m_pReply;
    dbus_uint32_t m_serial;
    DBusConnection* m_pConnection;
    DBusError m_err;
};


inline void DBUSMessageHandler::setConnection(DBusConnection* conn)
{
    m_pConnection = conn;
}

inline DBusConnection* DBUSMessageHandler::getConnection()
{
    return m_pConnection;
}

#endif // _DBUSMESSAGEWRAPPER_H_
