/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file ControlSender.cpp
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

#include "ControlSender.h"
#include "PluginTemplate.h"
#include "DLTWrapper.h"
#include <cassert>
#include <fstream>

using namespace am;

#define REQUIRED_MIN_INTERFACE_VERSION 1

ControlSender::ControlSender(std::string controlPluginFile) :
        mlibHandle(NULL), //
        mController(NULL)
{
    std::ifstream isfile(controlPluginFile.c_str());
    if (!isfile)
    {
        logError("ControlSender::ControlSender: Controller plugin not found:", controlPluginFile);
    }
    else if (!controlPluginFile.empty())
    {
        ControlSendInterface* (*createFunc)();
        createFunc = getCreateFunction<ControlSendInterface*()>(controlPluginFile, mlibHandle);
        assert(createFunc!=NULL);
        mController = createFunc();

        //check libversion
        assert(REQUIRED_MIN_INTERFACE_VERSION<=mController->getInterfaceVersion());
    }
    else
    {
        logError("ControlSender::ControlSender: No controller loaded !");
    }
}

ControlSender::~ControlSender()
{
    if (mlibHandle)
        dlclose(mlibHandle);
}

void ControlSender::hookAllPluginsLoaded()
{
    mController->hookAllPluginsLoaded();
}

am_Error_e ControlSender::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    return mController->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID);
}

am_Error_e ControlSender::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    return mController->hookUserDisconnectionRequest(connectionID);
}

am_Error_e ControlSender::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    return mController->hookUserSetMainSinkSoundProperty(sinkID, soundProperty);
}

am_Error_e ControlSender::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    return mController->hookUserSetMainSourceSoundProperty(sourceID, soundProperty);
}

am_Error_e ControlSender::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    return mController->hookUserSetSystemProperty(property);
}

am_Error_e ControlSender::hookUserVolumeChange(const am_sinkID_t sinkID, const am_mainVolume_t newVolume)
{
    return mController->hookUserVolumeChange(sinkID, newVolume);
}

am_Error_e ControlSender::hookUserVolumeStep(const am_sinkID_t sinkID, const int16_t increment)
{
    return mController->hookUserVolumeStep(sinkID, increment);
}

am_Error_e ControlSender::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    return mController->hookUserSetSinkMuteState(sinkID, muteState);
}

am_Error_e ControlSender::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    return mController->hookSystemRegisterDomain(domainData, domainID);
}

am_Error_e ControlSender::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    return mController->hookSystemDeregisterDomain(domainID);
}

void ControlSender::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    return mController->hookSystemDomainRegistrationComplete(domainID);
}

am_Error_e ControlSender::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    return mController->hookSystemRegisterSink(sinkData, sinkID);
}

am_Error_e ControlSender::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    return mController->hookSystemDeregisterSink(sinkID);
}

am_Error_e ControlSender::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    return mController->hookSystemRegisterSource(sourceData, sourceID);
}

am_Error_e ControlSender::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    return mController->hookSystemDeregisterSource(sourceID);
}

am_Error_e ControlSender::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    return mController->hookSystemRegisterGateway(gatewayData, gatewayID);
}

am_Error_e ControlSender::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    return mController->hookSystemDeregisterGateway(gatewayID);
}

am_Error_e ControlSender::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    return mController->hookSystemRegisterCrossfader(crossfaderData, crossfaderID);
}

am_Error_e ControlSender::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    return mController->hookSystemDeregisterCrossfader(crossfaderID);
}

void ControlSender::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    mController->hookSystemSinkVolumeTick(handle, sinkID, volume);
}

void ControlSender::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    mController->hookSystemSourceVolumeTick(handle, sourceID, volume);
}

void ControlSender::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    mController->hookSystemInterruptStateChange(sourceID, interruptState);
}

void ControlSender::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mController->hookSystemSinkAvailablityStateChange(sinkID, availability);
}

void ControlSender::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mController->hookSystemSourceAvailablityStateChange(sourceID, availability);
}

void ControlSender::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    mController->hookSystemDomainStateChange(domainID, state);
}

void ControlSender::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    mController->hookSystemReceiveEarlyData(data);
}

void ControlSender::hookSystemSpeedChange(const am_speed_t speed)
{
    mController->hookSystemSpeedChange(speed);
}

void ControlSender::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    mController->hookSystemTimingInformationChanged(mainConnectionID, time);
}

void ControlSender::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    mController->cbAckConnect(handle, errorID);
}

void ControlSender::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    mController->cbAckDisconnect(handle, errorID);
}

void ControlSender::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    mController->cbAckCrossFade(handle, hostsink, error);
}

void ControlSender::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    mController->cbAckSetSinkVolumeChange(handle, volume, error);
}

void ControlSender::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    mController->cbAckSetSourceVolumeChange(handle, volume, error);
}

void ControlSender::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSourceState(handle, error);
}

void ControlSender::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSourceSoundProperty(handle, error);
}

am_Error_e ControlSender::startupController(ControlReceiveInterface *controlreceiveinterface)
{
    return mController->startupController(controlreceiveinterface);
}

am_Error_e ControlSender::stopController()
{
    return mController->stopController();
}

void ControlSender::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSinkSoundProperty(handle, error);
}

void ControlSender::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSinkSoundProperties(handle, error);
}

void ControlSender::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSourceSoundProperties(handle, error);
}

am_Error_e am::ControlSender::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e> & listPrioConnectionFormats)
{
    return mController->getConnectionFormatChoice(sourceID, sinkID, listRoute, listPossibleConnectionFormats, listPrioConnectionFormats);
}

uint16_t ControlSender::getInterfaceVersion() const
{
    return ControlSendVersion;
}

