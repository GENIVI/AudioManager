/*
 * RoutingSender.h
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include <routing/RoutingSendInterface.h>

using namespace am;

class DbusRoutingSender: public RoutingSendInterface {
public:
	DbusRoutingSender();
	virtual ~DbusRoutingSender();
	void startupRoutingInterface(RoutingReceiveInterface* routingreceiveinterface) ;
	void routingInterfacesReady() ;
	void routingInterfacesRundown() ;
	am_Error_e asyncAbort(const am_Handle_s handle) ;
	am_Error_e asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) ;
	am_Error_e asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID) ;
	am_Error_e asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state) ;
	am_Error_e asyncSetSinkSoundProperty(const am_Handle_s handle, const am_SoundProperty_s& soundProperty, const am_sinkID_t sinkID) ;
	am_Error_e asyncSetSourceSoundProperty(const am_Handle_s handle, const am_SoundProperty_s& soundProperty, const am_sourceID_t sourceID) ;
	am_Error_e asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time) ;
	am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState) ;
	am_Error_e returnBusName(std::string& BusName) const ;
};

#endif /* ROUTINGSENDER_H_ */
