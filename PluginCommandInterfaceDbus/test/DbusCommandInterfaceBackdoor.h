/*
 * DbusCommandInterfaceBackdoor.h
 *
 *  Created on: Dec 16, 2011
 *      Author: christian
 */

#ifndef DBUSCOMMANDINTERFACEBACKDOOR_H_
#define DBUSCOMMANDINTERFACEBACKDOOR_H_

#include <dbus/dbus.h>
#include "../include/DBusCommandSender.h"
#include "../include/DBusMessageHandler.h"
#include "../include/CommandReceiverShadow.h"

namespace am {

class DbusCommandSender;

class DbusCommandInterfaceBackdoor
{
public:
	DbusCommandInterfaceBackdoor();
	virtual ~DbusCommandInterfaceBackdoor();
	void setReceiveInterface(DbusCommandSender *sender, CommandReceiveInterface* interface);
	void setDbusConnection(DbusCommandSender *sender,DBusConnection *conn);
	void setListSinks(DbusCommandSender *sender, std::vector<am_SinkType_s> newList);
	void setListSources(DbusCommandSender *sender, std::vector<am_SourceType_s> newList);
};

}

#endif /* DBUSCOMMANDINTERFACEBACKDOOR_H_ */
