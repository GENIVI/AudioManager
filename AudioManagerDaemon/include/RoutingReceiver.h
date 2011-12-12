/*
 * RoutingReceiver.h
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#ifndef ROUTINGRECEIVER_H_
#define ROUTINGRECEIVER_H_

#include "routing/RoutingReceiveInterface.h"
#include "ControlSender.h"
#include "RoutingSender.h"
#include "DatabaseHandler.h"

using namespace am;

class RoutingReceiver : public RoutingReceiveInterface {
public:
	RoutingReceiver(DatabaseHandler *iDatabaseHandler, RoutingSender *iRoutingSender, ControlSender *iControlSender);
	virtual ~RoutingReceiver();
	void ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error) ;
	void ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error) ;
	void ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error) ;
	void ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error) ;
	void ackSetSourceState(const am_Handle_s handle, const am_Error_e error) ;
	void ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error) ;
	void ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error) ;
	void ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error) ;
	void ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume) ;
	void ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume) ;
	am_Error_e peekDomain(const std::string& name, am_domainID_t& domainID) ;
	am_Error_e registerDomain(const am_Domain_s& domainData, am_domainID_t& domainID) ;
	am_Error_e deregisterDomain(const am_domainID_t domainID) ;
	am_Error_e registerGateway(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID) ;
	am_Error_e deregisterGateway(const am_gatewayID_t gatewayID) ;
	am_Error_e peekSink(const std::string& name, am_sinkID_t& sinkID) ;
	am_Error_e registerSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID) ;
	am_Error_e deregisterSink(const am_sinkID_t sinkID) ;
	am_Error_e peekSource(const std::string& name, am_sourceID_t& sourceID) ;
	am_Error_e registerSource(const am_Source_s& sourceData, am_sourceID_t& sourceID) ;
	am_Error_e deregisterSource(const am_sourceID_t sourceID) ;
	am_Error_e registerCrossfader(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID) ;
	am_Error_e deregisterCrossfader(const am_crossfaderID_t crossfaderID) ;
	am_Error_e peekSinkClassID(const std::string& name, am_sourceClass_t& sourceClassID) ;
	am_Error_e peekSourceClassID(const std::string& name, am_sinkClass_t& sinkClassID) ;
	void hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState) ;
	void hookDomainRegistrationComplete(const am_domainID_t domainID) ;
	void hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s& availability) ;
	void hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s& availability) ;
	void hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState) ;
	void hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay) ;
	am_Error_e sendChangedData(const std::vector<am_EarlyData_s>& earlyData) ;
	am_Error_e getDBusConnectionWrapper(DBusWrapper* dbusConnectionWrapper) const ;
private:
	DatabaseHandler *mDatabaseHandler;
	RoutingSender *mRoutingSender;
	ControlSender *mControlSender;

};

#endif /* ROUTINGRECEIVER_H_ */
