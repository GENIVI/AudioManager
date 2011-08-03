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
#ifndef _DBUSTYPES_H_
#define _DBUSTYPES_H_

#include "headers.h"

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

#endif // _DBUSTYPES_H_
