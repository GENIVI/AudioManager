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
#include "routinginterface.h"
#include "AudiomanagerInterface.h"
#include "DbusSend.h"
#include "DBUSMessageHandler.h"
#include "DBUSIntrospection.h"
#include "DbusInterface.h"


#define BUS_NAME "DBUS"
#define DBUS_BUSNAME "org.genivi.pulse"
#define DBUS_PATH "/pulse"

const char DBUS_SERVICE_PREFIX[] = "org.genivi.audiomanagerRouting\0";

DLT_IMPORT_CONTEXT(DBusPlugin);

#endif /* HEADERS_H_ */
