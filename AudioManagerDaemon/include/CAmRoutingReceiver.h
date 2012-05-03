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
 * \file CAmRoutingReceiver.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef ROUTINGRECEIVER_H_
#define ROUTINGRECEIVER_H_

#include "routing/IAmRoutingReceive.h"

namespace am
{

class CAmSocketHandler;
class CAmDbusWrapper;
class CAmDatabaseHandler;
class CAmRoutingSender;
class CAmControlSender;

/**
 * Implements the Receiving side of the RoutingPlugins.
 */
class CAmRoutingReceiver: public IAmRoutingReceive
{
public:
    CAmRoutingReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler);
    CAmRoutingReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler, CAmDbusWrapper *iDBusWrapper);
    ~CAmRoutingReceiver();
    void ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error);
    void ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error);
    void ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void ackSetSourceState(const am_Handle_s handle, const am_Error_e error);
    void ackSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error);
    void ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void ackSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error);
    void ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error);
    void ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume);
    void ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume);
    am_Error_e peekDomain(const std::string& name, am_domainID_t& domainID);
    am_Error_e registerDomain(const am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e deregisterDomain(const am_domainID_t domainID);
    am_Error_e registerGateway(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e deregisterGateway(const am_gatewayID_t gatewayID);
    am_Error_e peekSink(const std::string& name, am_sinkID_t& sinkID);
    am_Error_e registerSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e deregisterSink(const am_sinkID_t sinkID);
    am_Error_e peekSource(const std::string& name, am_sourceID_t& sourceID);
    am_Error_e registerSource(const am_Source_s& sourceData, am_sourceID_t& sourceID);
    am_Error_e deregisterSource(const am_sourceID_t sourceID);
    am_Error_e registerCrossfader(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e deregisterCrossfader(const am_crossfaderID_t crossfaderID);
    am_Error_e peekSourceClassID(const std::string& name, am_sourceClass_t& sourceClassID);
    am_Error_e peekSinkClassID(const std::string& name, am_sinkClass_t& sinkClassID);
    void hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState);
    void hookDomainRegistrationComplete(const am_domainID_t domainID);
    void hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState);
    void hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay);
    void sendChangedData(const std::vector<am_EarlyData_s>& earlyData);
    am_Error_e getDBusConnectionWrapper(CAmDbusWrapper*& dbusConnectionWrapper) const;
    am_Error_e getSocketHandler(CAmSocketHandler*& socketHandler) const;
    void getInterfaceVersion(std::string& version) const;
    void confirmRoutingReady(const uint16_t handle);
    void confirmRoutingRundown(const uint16_t handle);

    uint16_t getStartupHandle(); //!< returns a startup handle
    uint16_t getRundownHandle(); //!< returns a rundown handle

    void waitOnStartup(bool startup); //!< tells the RoutingReceiver to start waiting for all handles to be confirmed
    void waitOnRundown(bool rundown); //!< tells the RoutingReceiver to start waiting for all handles to be confirmed

private:
    CAmDatabaseHandler *mpDatabaseHandler; //!< pointer to the databaseHandler
    CAmRoutingSender *mpRoutingSender; //!< pointer to the routingSender
    CAmControlSender *mpControlSender; //!< pointer to the controlSender
    CAmSocketHandler *mpSocketHandler; //!< pointer to sockethandler
    CAmDbusWrapper *mpDBusWrapper; //!< pointer to dbuswrapper

    std::vector<uint16_t> mListStartupHandles; //!< list of handles that wait for a confirm
    std::vector<uint16_t> mListRundownHandles; //!< list of handles that wait for a confirm
    uint16_t handleCount; //!< counts all handles
    bool mWaitStartup; //!< if true confirmation will be sent if list of handles = 0
    bool mWaitRundown; //!< if true confirmation will be sent if list of handles = 0

};

}

#endif /* ROUTINGRECEIVER_H_ */
