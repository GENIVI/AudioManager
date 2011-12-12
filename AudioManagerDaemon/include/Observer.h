/*
 * Observer.h
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "CommandSender.h"
#include "RoutingSender.h"

class Observer {
public:
	Observer(CommandSender *iCommandSender, RoutingSender *iRoutingSender);
	virtual ~Observer();
	void numberOfMainConnectionsChanged() ;
	void numberOfSinkClassesChanged() ;
	void numberOfSourceClassesChanged() ;
	void newSink(am_Sink_s sink);
	void newSource(am_Source_s source);
	void newDomain(am_Domain_s domain);
	void newGateway(am_Gateway_s gateway);
	void newCrossfader(am_Crossfader_s crossfader);
	void removedSink(am_sinkID_t sinkID);
	void removedSource(am_sourceID_t sourceID);
	void removeDomain(am_domainID_t domainID);
	void removeGateway(am_gatewayID_t gatewayID);
	void removeCrossfader(am_crossfaderID_t crossfaderID);
	void mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState) ;
	void mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty) ;
	void mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty) ;
	void sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability) ;
	void sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability) ;
	void volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume) ;
	void sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState) ;
	void systemPropertyChanged(const am_SystemProperty_s& SystemProperty) ;
	void timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time) ;
private:
	CommandSender *mCommandSender;
	RoutingSender* mRoutingSender;
};

#endif /* OBSERVER_H_ */
