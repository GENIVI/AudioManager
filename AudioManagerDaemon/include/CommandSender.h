/*
 * CommandSender.h
 *
 *  Created on: Oct 26, 2011
 *      Author: christian
 */

#ifndef COMMANDSENDER_H_
#define COMMANDSENDER_H_

#include "pluginTemplate.h"
#include "command/CommandSendInterface.h"

using namespace am;

class CommandSender {
public:
	CommandSender();
	virtual ~CommandSender();
	am_Error_e startupInterface(CommandReceiveInterface* commandreceiveinterface) ;
	am_Error_e stopInterface() ;
	void cbCommunicationReady() ;
	void cbCommunicationRundown() ;
	void cbNumberOfMainConnectionsChanged() ;
	void cbNumberOfSinksChanged() ;
	void cbNumberOfSourcesChanged() ;
	void cbNumberOfSinkClassesChanged() ;
	void cbNumberOfSourceClassesChanged() ;
	void cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState) ;
	void cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty) ;
	void cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty) ;
	void cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability) ;
	void cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability) ;
	void cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume) ;
	void cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState) ;
	void cbSystemPropertyChanged(const am_SystemProperty_s& SystemProperty) ;
	void cbTimingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time) ;
private:
	std::vector<CommandSendInterface*> mListInterfaces;
};

#endif /* COMMANDSENDER_H_ */
