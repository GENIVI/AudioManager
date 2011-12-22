/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file ControlReceiver.h
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

#ifndef CONTRONLRECEIVER_H_
#define CONTRONLRECEIVER_H_

#include <control/ControlReceiveInterface.h>
#include <SocketHandler.h>
#include <config.h>
#ifdef WITH_DBUS_WRAPPER
#include <dbus/DBusWrapper.h>
#endif
#include "DatabaseHandler.h"
#include "RoutingSender.h"
#include "CommandSender.h"

namespace am {

/**
 * This class is used to receive all commands from the control interface
 */
class ControlReceiver: public ControlReceiveInterface {
public:
	ControlReceiver(DatabaseHandler *iDatabaseHandler, RoutingSender *iRoutingSender, CommandSender *iCommandSender, SocketHandler *iSocketHandler);
	ControlReceiver(DatabaseHandler *iDatabaseHandler, RoutingSender *iRoutingSender, CommandSender *iCommandSender);
	virtual ~ControlReceiver();
	am_Error_e getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList) ;
	am_Error_e connect(am_Handle_s& handle, am_connectionID_t& connectionID, const am_ConnectionFormat_e format, const am_sourceID_t sourceID, const am_sinkID_t sinkID) ;
	am_Error_e disconnect(am_Handle_s& handle, const am_connectionID_t connectionID) ;
	am_Error_e crossfade(am_Handle_s& handle, const am_HotSink_e hotSource, const am_crossfaderID_t crossfaderID, const am_RampType_e rampType, const am_time_t rampTime) ;
	am_Error_e setSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state) ;
	am_Error_e setSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
	am_Error_e setSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e rampType, const am_time_t time) ;
	am_Error_e setSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty) ;
	am_Error_e setSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty) ;
	am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState) ;
	am_Error_e abortAction(const am_Handle_s handle) ;
	am_Error_e enterDomainDB(const am_Domain_s& domainData, am_domainID_t& domainID) ;
	am_Error_e enterMainConnectionDB(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID) ;
	am_Error_e enterSinkDB(const am_Sink_s& sinkData, am_sinkID_t& sinkID) ;
	am_Error_e enterCrossfaderDB(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID) ;
	am_Error_e enterGatewayDB(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID) ;
	am_Error_e enterSourceDB(const am_Source_s& sourceData, am_sourceID_t& sourceID) ;
	am_Error_e enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID) ;
	am_Error_e enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass) ;
	am_Error_e enterSystemPropertiesListDB(const std::vector<am_SystemProperty_s>& listSystemProperties) ;
	am_Error_e changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const am_Route_s& route) ;
	am_Error_e changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState) ;
	am_Error_e changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID) ;
	am_Error_e changeSinkAvailabilityDB(const am_Availability_s& availability, const am_sinkID_t sinkID) ;
	am_Error_e changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID) ;
	am_Error_e changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID) ;
	am_Error_e changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID) ;
	am_Error_e changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID) ;
	am_Error_e changeSourceAvailabilityDB(const am_Availability_s& availability, const am_sourceID_t sourceID) ;
	am_Error_e changeSystemPropertyDB(const am_SystemProperty_s& property) ;
	am_Error_e changeSinkClassInfoDB(const am_SinkClass_s& classInfo) ;
	am_Error_e changeSourceClassInfoDB(const am_SourceClass_s& classInfo) ;
	am_Error_e removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID) ;
	am_Error_e removeSinkDB(const am_sinkID_t sinkID) ;
	am_Error_e removeSourceDB(const am_sourceID_t sourceID) ;
	am_Error_e removeGatewayDB(const am_gatewayID_t gatewayID) ;
	am_Error_e removeCrossfaderDB(const am_crossfaderID_t crossfaderID) ;
	am_Error_e removeDomainDB(const am_domainID_t domainID) ;
	am_Error_e removeSinkClassDB(const am_sinkClass_t sinkClassID) ;
	am_Error_e removeSourceClassDB(const am_sourceClass_t sourceClassID) ;
	am_Error_e getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s& classInfo) const ;
	am_Error_e getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass) const ;
	am_Error_e getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData) const ;
	am_Error_e getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData) const ;
	am_Error_e getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID) const ;
	am_Error_e getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID) const ;
	am_Error_e getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t>& listGatewaysID) const ;
	am_Error_e getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID) const ;
	am_Error_e getListMainConnections(std::vector<am_MainConnection_s>& listMainConnections) const ;
	am_Error_e getListDomains(std::vector<am_Domain_s>& listDomains) const ;
	am_Error_e getListConnections(std::vector<am_Connection_s>& listConnections) const ;
	am_Error_e getListSinks(std::vector<am_Sink_s>& listSinks) const ;
	am_Error_e getListSources(std::vector<am_Source_s>& lisSources) const ;
	am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const ;
	am_Error_e getListHandles(std::vector<am_Handle_s>& listHandles) const ;
	am_Error_e getListCrossfaders(std::vector<am_Crossfader_s>& listCrossfaders) const ;
	am_Error_e getListGateways(std::vector<am_Gateway_s>& listGateways) const ;
	am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const ;
	am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const ;
	void setRoutingReady() ;
	void setCommandReady() ;
	am_Error_e getSocketHandler(SocketHandler*& socketHandler);

private:
	DatabaseHandler* mDatabaseHandler; //!< pointer tto the databasehandler
	RoutingSender* mRoutingSender; //!< pointer to the routing send interface.
	CommandSender* mCommandSender; //!< pointer to the command send interface
	SocketHandler* mSocketHandler; //!< pointer to the socketHandler
};

}

#endif /* CONTRONLRECEIVER_H_ */
