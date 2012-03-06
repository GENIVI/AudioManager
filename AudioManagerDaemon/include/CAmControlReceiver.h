/**
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file CAmControlReceiver.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef CONTRONLRECEIVER_H_
#define CONTRONLRECEIVER_H_

#include "control/IAmControlReceive.h"

namespace am
{

class CAmSocketHandler;
class CAmDatabaseHandler;
class CAmRoutingSender;
class CAmCommandSender;
class CAmRouter;

/**
 * This class is used to receive all commands from the control interface
 */
class CAmControlReceiver: public IAmControlReceive
{
public:
    CAmControlReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmCommandSender *iCommandSender, CAmSocketHandler *iSocketHandler, CAmRouter* iRouter);
    ~CAmControlReceiver();
    am_Error_e getRoute(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s>& returnList);
    am_Error_e connect(am_Handle_s& handle, am_connectionID_t& connectionID, const am_ConnectionFormat_e format, const am_sourceID_t sourceID, const am_sinkID_t sinkID);
    am_Error_e disconnect(am_Handle_s& handle, const am_connectionID_t connectionID);
    am_Error_e crossfade(am_Handle_s& handle, const am_HotSink_e hotSource, const am_crossfaderID_t crossfaderID, const am_RampType_e rampType, const am_time_t rampTime);
    am_Error_e abortAction(const am_Handle_s handle);
    am_Error_e setSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state);
    am_Error_e setSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time);
    am_Error_e setSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e rampType, const am_time_t time);
    am_Error_e setSinkSoundProperties(am_Handle_s& handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& soundProperty);
    am_Error_e setSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty);
    am_Error_e setSourceSoundProperties(am_Handle_s& handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& soundProperty);
    am_Error_e setSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty);
    am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState);
    am_Error_e enterDomainDB(const am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e enterMainConnectionDB(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID);
    am_Error_e enterSinkDB(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e enterCrossfaderDB(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e enterGatewayDB(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e enterSourceDB(const am_Source_s& sourceData, am_sourceID_t& sourceID);
    am_Error_e enterSinkClassDB(const am_SinkClass_s& sinkClass, am_sinkClass_t& sinkClassID);
    am_Error_e enterSourceClassDB(am_sourceClass_t& sourceClassID, const am_SourceClass_s& sourceClass);
    am_Error_e changeSinkClassInfoDB(const am_SinkClass_s& sinkClass);
    am_Error_e changeSourceClassInfoDB(const am_SourceClass_s& sourceClass);
    am_Error_e enterSystemPropertiesListDB(const std::vector<am_SystemProperty_s>& listSystemProperties);
    am_Error_e changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID);
    am_Error_e changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState);
    am_Error_e changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID);
    am_Error_e changeSinkAvailabilityDB(const am_Availability_s& availability, const am_sinkID_t sinkID);
    am_Error_e changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID);
    am_Error_e changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID);
    am_Error_e changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID);
    am_Error_e changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID);
    am_Error_e changeSourceAvailabilityDB(const am_Availability_s& availability, const am_sourceID_t sourceID);
    am_Error_e changeSystemPropertyDB(const am_SystemProperty_s& property);
    am_Error_e removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID);
    am_Error_e removeSinkDB(const am_sinkID_t sinkID);
    am_Error_e removeSourceDB(const am_sourceID_t sourceID);
    am_Error_e removeGatewayDB(const am_gatewayID_t gatewayID);
    am_Error_e removeCrossfaderDB(const am_crossfaderID_t crossfaderID);
    am_Error_e removeDomainDB(const am_domainID_t domainID);
    am_Error_e removeSinkClassDB(const am_sinkClass_t sinkClassID);
    am_Error_e removeSourceClassDB(const am_sourceClass_t sourceClassID);
    am_Error_e getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s& classInfo) const;
    am_Error_e getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass) const;
    am_Error_e getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s& sinkData) const;
    am_Error_e getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s& sourceData) const;
    am_Error_e getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData) const;
    am_Error_e getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData) const;
    am_Error_e getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s& mainConnectionData) const;
    am_Error_e getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID) const;
    am_Error_e getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID) const;
    am_Error_e getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t>& listCrossfadersID) const;
    am_Error_e getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID) const;
    am_Error_e getListMainConnections(std::vector<am_MainConnection_s>& listMainConnections) const;
    am_Error_e getListDomains(std::vector<am_Domain_s>& listDomains) const;
    am_Error_e getListConnections(std::vector<am_Connection_s>& listConnections) const;
    am_Error_e getListSinks(std::vector<am_Sink_s>& listSinks) const;
    am_Error_e getListSources(std::vector<am_Source_s>& listSources) const;
    am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const;
    am_Error_e getListHandles(std::vector<am_Handle_s>& listHandles) const;
    am_Error_e getListCrossfaders(std::vector<am_Crossfader_s>& listCrossfaders) const;
    am_Error_e getListGateways(std::vector<am_Gateway_s>& listGateways) const;
    am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const;
    am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const;
    void setCommandReady();
    void setCommandRundown();
    void setRoutingReady();
    void setRoutingRundown();
    void confirmControllerReady();
    void confirmControllerRundown();
    am_Error_e getSocketHandler(CAmSocketHandler*& socketHandler);
    void getInterfaceVersion(std::string& version) const;

private:
    CAmDatabaseHandler* mDatabaseHandler; //!< pointer tto the databasehandler
    CAmRoutingSender* mRoutingSender; //!< pointer to the routing send interface.
    CAmCommandSender* mCommandSender; //!< pointer to the command send interface
    CAmSocketHandler* mSocketHandler; //!< pointer to the socketHandler
    CAmRouter* mRouter; //!< pointer to the Router
};

}

#endif /* CONTRONLRECEIVER_H_ */
