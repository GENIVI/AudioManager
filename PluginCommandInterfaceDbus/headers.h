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
#include "DBUSIntrospection.h"
#include "DbusInterface.h"
#include "DBUSTypes.h"


#define BUS_NAME "DBUS"
#define DBUS_BUSNAME "org.genivi.pulse"
#define DBUS_PATH "/pulse"

const char DBUS_SERVICE_PREFIX[] = "org.bla.audiomanagerCommand\0";
const char DBUS_SERVICE_PREFIX_PATH[] = "/org/bla/audiomanagerCommand\0";

DLT_IMPORT_CONTEXT(DBusCommandPlugin);

#endif /* HEADERS_H_ */
