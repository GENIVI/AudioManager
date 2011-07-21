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

#ifndef _DBUSINTROSPECTION_H_
#define _DBUSINTROSPECTION_H_

#include <sstream>
using std::stringstream;

#include <string>
using std::string;

#include "DBUSTypes.h"
#include <dbus/dbus.h>

class DBUSIntrospection
{
public:
    DBUSIntrospection(MethodTable* table);
    void process(DBusConnection* conn, DBusMessage* msg);

private:
    void generateString(void);

    void addHeader(void);
    void addArgument(string argname, string direction, string type);
    void addEntry(MethodTable entry);

    void openNode(string nodename);
    void closeNode(void);

    void openInterface(string interfacename);
    void closeInterface(void);

    void openMethod(string methodname);
    void closeMethod(void);


private:
    stringstream m_introspectionString;
    MethodTable* m_methodTable;
};


#endif // _DBUSINTROSPECTION_H_
