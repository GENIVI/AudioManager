/*
 * headers.h
 *
 *  Created on: Jul 21, 2011
 *      Author: christian
 */

#ifndef HEADERS_H_
#define HEADERS_H_

#include <dbus/dbus.h>
#include <dlt/dlt.h>
#include "CommandReceive.h"
#include "CommandDbusReceive.h"
#include "commandInterface.h"
#include "DbusSend.h"
#include "DBUSMessageHandler.h"
#include "dbusRoothandler.h"
#include "DbusInterface.h"


const char MY_NODE[]="commandinterface\0";

DLT_IMPORT_CONTEXT(DBusCommandPlugin);

#endif /* HEADERS_H_ */
