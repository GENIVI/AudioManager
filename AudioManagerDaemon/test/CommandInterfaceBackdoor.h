/*
 * CommandInterfaceBackdoor.h
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#ifndef COMMANDINTERFACEBACKDOOR_H_
#define COMMANDINTERFACEBACKDOOR_H_

#include <command/CommandSendInterface.h>
#include "CommandSender.h"

using namespace am;

class CommandSender;

class CommandInterfaceBackdoor {
public:
	CommandInterfaceBackdoor();
	virtual ~CommandInterfaceBackdoor();
	bool unloadPlugins(CommandSender *CommandSender);
	bool injectInterface(CommandSender* CommandSender, CommandSendInterface* CommandSendInterface);
};

//definitions are in CommonFunctions.cpp!

#endif /* COMMANDINTERFACEBACKDOOR_H_ */
