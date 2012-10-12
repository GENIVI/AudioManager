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
 * \file CAmControlSender.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef CONTROLSENDER_H_
#define CONTROLSENDER_H_

#ifdef UNIT_TEST
#include "../test/IAmControlBackdoor.h"
#endif

#include "control/IAmControlSend.h"

namespace am
{

/**
 * sends data to the commandInterface, takes the file of the library that needs to be loaded
 */
class CAmControlSender
{
public:
    CAmControlSender(std::string controlPluginFile);
    ~CAmControlSender();
    am_Error_e startupController(IAmControlReceive* controlreceiveinterface) ;
    void setControllerReady() ;
    void setControllerRundown() ;
    am_Error_e hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t& mainConnectionID) ;
    am_Error_e hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID) ;
    am_Error_e hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty) ;
    am_Error_e hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty) ;
    am_Error_e hookUserSetSystemProperty(const am_SystemProperty_s& property) ;
    am_Error_e hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume) ;
    am_Error_e hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment) ;
    am_Error_e hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState) ;
    am_Error_e hookSystemRegisterDomain(const am_Domain_s& domainData, am_domainID_t& domainID) ;
    am_Error_e hookSystemDeregisterDomain(const am_domainID_t domainID) ;
    void hookSystemDomainRegistrationComplete(const am_domainID_t domainID) ;
    am_Error_e hookSystemRegisterSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID) ;
    am_Error_e hookSystemDeregisterSink(const am_sinkID_t sinkID) ;
    am_Error_e hookSystemRegisterSource(const am_Source_s& sourceData, am_sourceID_t& sourceID) ;
    am_Error_e hookSystemDeregisterSource(const am_sourceID_t sourceID) ;
    am_Error_e hookSystemRegisterGateway(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID) ;
    am_Error_e hookSystemDeregisterGateway(const am_gatewayID_t gatewayID) ;
    am_Error_e hookSystemRegisterCrossfader(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID) ;
    am_Error_e hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID) ;
    void hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume) ;
    void hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume) ;
    void hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState) ;
    void hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s& availability) ;
    void hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s& availability) ;
    void hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state) ;
    void hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s>& data) ;
    void hookSystemSpeedChange(const am_speed_t speed) ;
    void hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time) ;
    void cbAckConnect(const am_Handle_s handle, const am_Error_e errorID) ;
    void cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID) ;
    void cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error) ;
    void cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error) ;
    void cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error) ;
    void cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error) ;
    void cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error) ;
    void cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error) ;
    void cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error) ;
    void cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error) ;
    am_Error_e getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e>& listPrioConnectionFormats) ;
    void getInterfaceVersion(std::string& version) const ;
    void confirmCommandReady() ;
    void confirmRoutingReady() ;
    void confirmCommandRundown() ;
    void confirmRoutingRundown() ;
    static void CallsetControllerRundown()
    {
        if (mInstance)
            mInstance->setControllerRundown();
    }

#ifdef UNIT_TEST
    friend class IAmControlBackdoor;
#endif
private:
    void* mlibHandle; //!< pointer to the loaded control plugin interface
    IAmControlSend* mController; //!< pointer to the ControlSend interface
    static CAmControlSender* mInstance;
};

}

#endif /* CONTROLSENDER_H_ */
