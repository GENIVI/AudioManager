/*
 * CommandReceiver.h
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#ifndef COMMANDRECEIVER_H_
#define COMMANDRECEIVER_H_


#include <command/CommandReceiveInterface.h>
#include "DatabaseHandler.h"
#include "DBusWrapper.h"

using namespace am;

class CommandReceiver: public  CommandReceiveInterface {
public:
	CommandReceiver(DatabaseHandler* iDatabaseHandler, DBusWrapper* iDBusWrapper);
	virtual ~CommandReceiver();
	am_Error_e connect(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t& mainConnectionID) ;
	am_Error_e disconnect(const am_mainConnectionID_t mainConnectionID) ;
	am_Error_e setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume) ;
	am_Error_e volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep) ;
	am_Error_e setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState) ;
	am_Error_e setMainSinkSoundProperty(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID) ;
	am_Error_e setMainSourceSoundProperty(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID) ;
	am_Error_e setSystemProperty(const am_SystemProperty_s& property) ;
	am_Error_e getListMainConnections(std::vector<am_MainConnectionType_s>& listConnections) const ;
	am_Error_e getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const ;
	am_Error_e getListMainSources(std::vector<am_SourceType_s>& listMainSources) const ;
	am_Error_e getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s>& listSoundProperties) const ;
	am_Error_e getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s>& listSourceProperties) const ;
	am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const ;
	am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const ;
	am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const ;
	am_Error_e getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay) const ;
	am_Error_e getDBusConnectionWrapper(DBusWrapper*& dbusConnectionWrapper) const ;

private:
	DatabaseHandler* mDatabaseHandler;
	DBusWrapper* mDBusWrapper;
};

#endif /* COMMANDRECEIVER_H_ */
