/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file ControlSender.h
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

#ifndef CONTROLSENDER_H_
#define CONTROLSENDER_H_

#ifdef UNIT_TEST
#include "../test/ControlInterfaceBackdoor.h"
#endif

#include "control/ControlSendInterface.h"

namespace am
{

/**
 * sends data to the commandInterface, takes the file of the library that needs to be loaded
 */
class ControlSender
{
public:
    ControlSender(std::string controlPluginFile);
    virtual ~ControlSender();
    am_Error_e startupController(ControlReceiveInterface* controlreceiveinterface);
    am_Error_e stopController();
    am_Error_e hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t& mainConnectionID);
    am_Error_e hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID);
    am_Error_e hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty);
    am_Error_e hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty);
    am_Error_e hookUserSetSystemProperty(const am_SystemProperty_s& property);
    am_Error_e hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume);
    am_Error_e hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment);
    am_Error_e hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    am_Error_e hookSystemRegisterDomain(const am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e hookSystemDeregisterDomain(const am_domainID_t domainID);
    am_Error_e hookSystemRegisterSink(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e hookSystemDeregisterSink(const am_sinkID_t sinkID);
    am_Error_e hookSystemRegisterSource(const am_Source_s& sourceData, am_sourceID_t& sourceID);
    am_Error_e hookSystemDeregisterSource(const am_sourceID_t sourceID);
    am_Error_e hookSystemRegisterGateway(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e hookSystemDeregisterGateway(const am_gatewayID_t gatewayID);
    am_Error_e hookSystemRegisterCrossfader(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID);
    void hookAllPluginsLoaded();
    void hookSystemDomainRegistrationComplete(const am_domainID_t domainID);
    void hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume);
    void hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume);
    void hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState);
    void hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state);
    void hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s>& data);
    void hookSystemSpeedChange(const am_speed_t speed);
    void hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time);
    void cbAckConnect(const am_Handle_s handle, const am_Error_e errorID);
    void cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID);
    void cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error);
    void cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error);
    am_Error_e getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e>& listPrioConnectionFormats);
    uint16_t getInterfaceVersion() const;

#ifdef UNIT_TEST
    friend class ControlInterfaceBackdoor;
#endif
private:
    void* mlibHandle; //!< pointer to the loaded control plugin interface
    ControlSendInterface* mController; //!< pointer to the ControlSend interface
};

}

#endif /* CONTROLSENDER_H_ */
