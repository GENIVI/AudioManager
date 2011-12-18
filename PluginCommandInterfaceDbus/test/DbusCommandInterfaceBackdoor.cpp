/*
 * DbusCommandInterfaceBackdoor.cpp
 *
 *  Created on: Dec 16, 2011
 *      Author: christian
 */

#include "DbusCommandInterfaceBackdoor.h"


using namespace am;

DbusCommandInterfaceBackdoor::DbusCommandInterfaceBackdoor()
{
}

DbusCommandInterfaceBackdoor::~DbusCommandInterfaceBackdoor()
{
}

void DbusCommandInterfaceBackdoor::setReceiveInterface(DbusCommandSender *sender, CommandReceiveInterface* interface)
{
	sender->mCommandReceiveInterface=interface;
}

void DbusCommandInterfaceBackdoor::setDbusConnection(DbusCommandSender *sender, DBusConnection *conn)
{
	sender->mDBUSMessageHandler.setDBusConnection(conn);
}

void DbusCommandInterfaceBackdoor::setListSinks(DbusCommandSender *sender, std::vector<am_SinkType_s> newList)
{
	sender->mlistSinks=newList;
}

void DbusCommandInterfaceBackdoor::setListSources(DbusCommandSender *sender, std::vector<am_SourceType_s> newList)
{
	sender->mlistSources=newList;
}













