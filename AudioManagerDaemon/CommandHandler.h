/*
 * CommandHandler.h
 *
 *  Created on: Jul 27, 2011
 *      Author: christian
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "audioManagerIncludes.h"
#include "commandInterface.h"

class CommandReceiveInterface;
class CommandSendInterface;


class CommandHandler {
public:
	CommandHandler(AudioManagerCore* core);
	void registerReceiver(CommandReceiveInterface* iface);
	virtual ~CommandHandler();
	void loadPlugins();
private:
	AudioManagerCore* m_core;
	CommandReceiveInterface* m_receiver;
	std::list<CommandSendInterface*> m_interfaceList;
};

#endif /* COMMANDHANDLER_H_ */
