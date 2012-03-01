/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file CAmControlSender.cpp
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

#include "CAmControlSender.h"
#include "TAmPluginTemplate.h"
#include "shared/CAmDltWrapper.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace am {

#define REQUIRED_INTERFACE_VERSION_MAJOR 1
#define REQUIRED_INTERFACE_VERSION_MINOR 0

CAmControlSender::CAmControlSender(std::string controlPluginFile) :
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
        IAmControlSend* (*createFunc)();
        createFunc = getCreateFunction<IAmControlSend*()>(controlPluginFile, mlibHandle);
        assert(createFunc!=NULL);
        mController = createFunc();

        //check libversion
        std::string version;
        mController->getInterfaceVersion(version);
        uint16_t minorVersion, majorVersion;
        std::istringstream(version.substr(0, 1)) >> majorVersion;
        std::istringstream(version.substr(2, 1)) >> minorVersion;

        if (majorVersion < REQUIRED_INTERFACE_VERSION_MAJOR || ((majorVersion == REQUIRED_INTERFACE_VERSION_MAJOR) && (minorVersion > REQUIRED_INTERFACE_VERSION_MINOR)))
        {
            logError("ControlSender::ControlSender: Interface Version of Controller too old, exiting now");
            throw std::runtime_error("Interface Version of Controller too old");
        }
    }
    else
    {
        logError("ControlSender::ControlSender: No controller loaded !");
    }
}

CAmControlSender::~CAmControlSender()
{
    if (mlibHandle)
        dlclose(mlibHandle);
}

am_Error_e CAmControlSender::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    return mController->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID);
}

am_Error_e CAmControlSender::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    return mController->hookUserDisconnectionRequest(connectionID);
}

am_Error_e CAmControlSender::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    return mController->hookUserSetMainSinkSoundProperty(sinkID, soundProperty);
}

am_Error_e CAmControlSender::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    return mController->hookUserSetMainSourceSoundProperty(sourceID, soundProperty);
}

am_Error_e CAmControlSender::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    return mController->hookUserSetSystemProperty(property);
}

am_Error_e CAmControlSender::hookUserVolumeChange(const am_sinkID_t sinkID, const am_mainVolume_t newVolume)
{
    return mController->hookUserVolumeChange(sinkID, newVolume);
}

am_Error_e CAmControlSender::hookUserVolumeStep(const am_sinkID_t sinkID, const int16_t increment)
{
    return mController->hookUserVolumeStep(sinkID, increment);
}

am_Error_e CAmControlSender::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    return mController->hookUserSetSinkMuteState(sinkID, muteState);
}

am_Error_e CAmControlSender::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    return mController->hookSystemRegisterDomain(domainData, domainID);
}

am_Error_e CAmControlSender::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    return mController->hookSystemDeregisterDomain(domainID);
}

void CAmControlSender::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    return mController->hookSystemDomainRegistrationComplete(domainID);
}

am_Error_e CAmControlSender::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    return mController->hookSystemRegisterSink(sinkData, sinkID);
}

am_Error_e CAmControlSender::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    return mController->hookSystemDeregisterSink(sinkID);
}

am_Error_e CAmControlSender::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    return mController->hookSystemRegisterSource(sourceData, sourceID);
}

am_Error_e CAmControlSender::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    return mController->hookSystemDeregisterSource(sourceID);
}

am_Error_e CAmControlSender::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    return mController->hookSystemRegisterGateway(gatewayData, gatewayID);
}

am_Error_e CAmControlSender::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    return mController->hookSystemDeregisterGateway(gatewayID);
}

am_Error_e CAmControlSender::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    return mController->hookSystemRegisterCrossfader(crossfaderData, crossfaderID);
}

am_Error_e CAmControlSender::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    return mController->hookSystemDeregisterCrossfader(crossfaderID);
}

void CAmControlSender::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    mController->hookSystemSinkVolumeTick(handle, sinkID, volume);
}

void CAmControlSender::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    mController->hookSystemSourceVolumeTick(handle, sourceID, volume);
}

void CAmControlSender::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    mController->hookSystemInterruptStateChange(sourceID, interruptState);
}

void CAmControlSender::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mController->hookSystemSinkAvailablityStateChange(sinkID, availability);
}

void CAmControlSender::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mController->hookSystemSourceAvailablityStateChange(sourceID, availability);
}

void CAmControlSender::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    mController->hookSystemDomainStateChange(domainID, state);
}

void CAmControlSender::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    mController->hookSystemReceiveEarlyData(data);
}

void CAmControlSender::hookSystemSpeedChange(const am_speed_t speed)
{
    mController->hookSystemSpeedChange(speed);
}

void CAmControlSender::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    mController->hookSystemTimingInformationChanged(mainConnectionID, time);
}

void CAmControlSender::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    mController->cbAckConnect(handle, errorID);
}

void CAmControlSender::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    mController->cbAckDisconnect(handle, errorID);
}

void CAmControlSender::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    mController->cbAckCrossFade(handle, hostsink, error);
}

void CAmControlSender::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    mController->cbAckSetSinkVolumeChange(handle, volume, error);
}

void CAmControlSender::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    mController->cbAckSetSourceVolumeChange(handle, volume, error);
}

void CAmControlSender::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSourceState(handle, error);
}

void CAmControlSender::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSourceSoundProperty(handle, error);
}

am_Error_e CAmControlSender::startupController(IAmControlReceive *controlreceiveinterface)
{
    return mController->startupController(controlreceiveinterface);
}

void CAmControlSender::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSinkSoundProperty(handle, error);
}

void CAmControlSender::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSinkSoundProperties(handle, error);
}

void CAmControlSender::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    mController->cbAckSetSourceSoundProperties(handle, error);
}

void CAmControlSender::setControllerReady()
{
    mController->setControllerReady();
}

void CAmControlSender::setControllerRundown()
{
    mController->setControllerRundown();
}

am_Error_e am::CAmControlSender::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e> & listPrioConnectionFormats)
{
    return mController->getConnectionFormatChoice(sourceID, sinkID, listRoute, listPossibleConnectionFormats, listPrioConnectionFormats);
}

void CAmControlSender::getInterfaceVersion(std::string & version) const
{
    version = ControlSendVersion;
}

void CAmControlSender::confirmCommandReady()
{
    mController->confirmCommandReady();
}

void CAmControlSender::confirmRoutingReady()
{
    mController->confirmRoutingReady();
}

void CAmControlSender::confirmCommandRundown()
{
    mController->confirmCommandRundown();
}

void CAmControlSender::confirmRoutingRundown()
{
    mController->confirmRoutingRundown();
}
}
