/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file RoutingReceiver.h
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
*/

#ifndef ROUTINGRECEIVER_H_
#define ROUTINGRECEIVER_H_

#include "routing/RoutingReceiveInterface.h"
#include "DatabaseHandler.h"
#include "RoutingSender.h"
#include "ControlSender.h"

using namespace am;

/**
 * Implements the Receiving side of the RoutingPlugins.
 */
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
	DatabaseHandler *mDatabaseHandler; //!< pointer to the databaseHandler
	RoutingSender *mRoutingSender; //!< pointer to the routingSender
	ControlSender *mControlSender; //!< pointer to the controlSender

};

#endif /* ROUTINGRECEIVER_H_ */
