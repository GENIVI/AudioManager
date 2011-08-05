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
#include "dbusRoothandler.h"
#include "AudiomanagerInterface.h"
#include "DbusSend.h"
#include "DBUSMessageHandler.h"
#include "DbusInterface.h"

#define BUS_NAME "DBUS"
#define DBUS_BUSNAME "org.genivi.command"
#define DBUS_PATH "/pulse"


const char MY_NODE[]="routinginterface\0";

DLT_IMPORT_CONTEXT(DBusPlugin);

#endif /* HEADERS_H_ */
