/*
 * CommandReceive.h
 *
 *  Created on: Jul 26, 2011
 *      Author: christian
 */

#ifndef COMMANDRECEIVE_H_
#define COMMANDRECEIVE_H_

#include "commandInterface.h"
#include "audioManagerIncludes.h"

class CommandReceive : CommandReceiveInterface {
public:
	CommandReceive(AudioManagerCore* core);
	virtual ~CommandReceive();
	connection_t connect(source_t source, sink_t sink);
	connection_t disconnect(source_t source, source_t sink);
	std::list<ConnectionType> getListConnections();
	std::list<SinkType> getListSinks();
	std::list<SourceType> getListSources();
	genInt_t interruptRequest(const std::string &SourceName, const std::string &SinkName);
	interrupt_t interruptResume(interrupt_t interrupt);
	volume_t setVolume(sink_t sink, volume_t volume);
private:
	AudioManagerCore* m_core;
};

#endif /* COMMANDRECEIVE_H_ */
