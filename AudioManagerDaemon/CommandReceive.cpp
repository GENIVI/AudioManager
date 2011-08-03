/*
 * CommandReceive.cpp
 *
 *  Created on: Jul 26, 2011
 *      Author: christian
 */

#include "CommandReceive.h"


CommandReceive::CommandReceive(){
}

CommandReceive::~CommandReceive() {

}

void CommandReceive::registerAudiomanagerCore(AudioManagerCore *core){
	m_core=core;
}


connection_t CommandReceive::connect(source_t source, sink_t sink) {
	DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("Connect"));
	if (m_core->UserConnect(source, sink) == GEN_OK) {
		return 1;
	}
	return -1;

	/**
	 * \todo returns a connection_t but this is always one. Somethings not right....
	 */
}

connection_t CommandReceive::disconnect(source_t source, source_t sink) {

	genRoute_t ReturnRoute;
	if (int value=m_core->returnDatabaseHandler()->returnMainconnectionIDforSinkSourceID(sink,source)>0) {
		return m_core->UserDisconnect(value);
	} else {
		return -1;
	}

	/**
	 * \todo not sure if the test of existance of thatconnection should be done here...
	 */
}

std::list<ConnectionType> CommandReceive::getListConnections() {
	return m_core->getListConnections();
}

std::list<SinkType> CommandReceive::getListSinks() {
	return m_core->getListSinks();
}

std::list<SourceType> CommandReceive::getListSources() {
	return m_core->getListSources();
}

genInt_t CommandReceive::interruptRequest(const std::string & SourceName,
		const std::string & SinkName) {
	source_t sourceID = m_core->returnSourceIDfromName(SourceName);
	sink_t sinkID = m_core->returnSinkIDfromName(SinkName);
	genInt_t intID = -1;
	if (m_core->interruptRequest(sourceID, sinkID, &intID) == GEN_OK) {
		return intID;
	}

	return -1;

	/**
	 * \todo need to change something?
	 */
}

interrupt_t CommandReceive::interruptResume(interrupt_t interrupt) {
	/**
	 * \todo here a callback mechanism needs to be installed....
	 */

	return 1;
}

volume_t CommandReceive::setVolume(sink_t sink, volume_t volume) {
	if (m_core->UserSetVolume(sink, volume) == GEN_OK) {
		return 1;
	}
	return -1;
}




