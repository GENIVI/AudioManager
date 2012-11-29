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
 * \file CAmControlSender.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmControlSender.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "TAmPluginTemplate.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

#define REQUIRED_INTERFACE_VERSION_MAJOR 1  //!< major interface version. All versions smaller than this will be rejected
#define REQUIRED_INTERFACE_VERSION_MINOR 0 //!< minor interface version. All versions smaller than this will be rejected

CAmControlSender* CAmControlSender::mInstance=NULL;

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
        mInstance=this;
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
    //if (mlibHandle)
    //    dlclose(mlibHandle);
}

am_Error_e CAmControlSender::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    assert(mController);
    return (mController->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));
}

am_Error_e CAmControlSender::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    assert(mController);
    return (mController->hookUserDisconnectionRequest(connectionID));
}

am_Error_e CAmControlSender::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    assert(mController);
    return (mController->hookUserSetMainSinkSoundProperty(sinkID, soundProperty));
}

am_Error_e CAmControlSender::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    assert(mController);
    return (mController->hookUserSetMainSourceSoundProperty(sourceID, soundProperty));
}

am_Error_e CAmControlSender::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    assert(mController);
    return (mController->hookUserSetSystemProperty(property));
}

am_Error_e CAmControlSender::hookUserVolumeChange(const am_sinkID_t sinkID, const am_mainVolume_t newVolume)
{
    assert(mController);
    return (mController->hookUserVolumeChange(sinkID, newVolume));
}

am_Error_e CAmControlSender::hookUserVolumeStep(const am_sinkID_t sinkID, const int16_t increment)
{
    assert(mController);
    return (mController->hookUserVolumeStep(sinkID, increment));
}

am_Error_e CAmControlSender::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    assert(mController);
    return (mController->hookUserSetSinkMuteState(sinkID, muteState));
}

am_Error_e CAmControlSender::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    assert(mController);
    return (mController->hookSystemRegisterDomain(domainData, domainID));
}

am_Error_e CAmControlSender::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    assert(mController);
    return (mController->hookSystemDeregisterDomain(domainID));
}

void CAmControlSender::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    assert(mController);
    return (mController->hookSystemDomainRegistrationComplete(domainID));
}

am_Error_e CAmControlSender::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    assert(mController);
    return (mController->hookSystemRegisterSink(sinkData, sinkID));
}

am_Error_e CAmControlSender::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    assert(mController);
    return (mController->hookSystemDeregisterSink(sinkID));
}

am_Error_e CAmControlSender::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    assert(mController);
    return (mController->hookSystemRegisterSource(sourceData, sourceID));
}

am_Error_e CAmControlSender::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    assert(mController);
    return (mController->hookSystemDeregisterSource(sourceID));
}

am_Error_e CAmControlSender::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    assert(mController);
    return (mController->hookSystemRegisterGateway(gatewayData, gatewayID));
}

am_Error_e CAmControlSender::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    assert(mController);
    return (mController->hookSystemDeregisterGateway(gatewayID));
}

am_Error_e CAmControlSender::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    assert(mController);
    return (mController->hookSystemRegisterCrossfader(crossfaderData, crossfaderID));
}

am_Error_e CAmControlSender::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    assert(mController);
    return (mController->hookSystemDeregisterCrossfader(crossfaderID));
}

void CAmControlSender::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    assert(mController);
    mController->hookSystemSinkVolumeTick(handle, sinkID, volume);
}

void CAmControlSender::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    assert(mController);
    mController->hookSystemSourceVolumeTick(handle, sourceID, volume);
}

void CAmControlSender::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    assert(mController);
    mController->hookSystemInterruptStateChange(sourceID, interruptState);
}

void CAmControlSender::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    assert(mController);
    mController->hookSystemSinkAvailablityStateChange(sinkID, availability);
}

void CAmControlSender::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    assert(mController);
    mController->hookSystemSourceAvailablityStateChange(sourceID, availability);
}

void CAmControlSender::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    assert(mController);
    mController->hookSystemDomainStateChange(domainID, state);
}

void CAmControlSender::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    assert(mController);
    mController->hookSystemReceiveEarlyData(data);
}

void CAmControlSender::hookSystemSpeedChange(const am_speed_t speed)
{
    assert(mController);
    mController->hookSystemSpeedChange(speed);
}

void CAmControlSender::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    assert(mController);
    mController->hookSystemTimingInformationChanged(mainConnectionID, time);
}

void CAmControlSender::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    assert(mController);
    mController->cbAckConnect(handle, errorID);
}

void CAmControlSender::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    assert(mController);
    mController->cbAckDisconnect(handle, errorID);
}

void CAmControlSender::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    assert(mController);
    mController->cbAckCrossFade(handle, hostsink, error);
}

void CAmControlSender::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkVolumeChange(handle, volume, error);
}

void CAmControlSender::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceVolumeChange(handle, volume, error);
}

void CAmControlSender::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceState(handle, error);
}

void CAmControlSender::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceSoundProperty(handle, error);
}

am_Error_e CAmControlSender::startupController(IAmControlReceive *controlreceiveinterface)
{
    if (!mController)
    {
        logError("ControlSender::startupController: no Controller to startup!");
        throw std::runtime_error("ControlSender::startupController: no Controller to startup! Exiting now ...");
        return (E_NON_EXISTENT);
    }
    return (mController->startupController(controlreceiveinterface));
}

void CAmControlSender::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkSoundProperty(handle, error);
}

void CAmControlSender::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSinkSoundProperties(handle, error);
}

void CAmControlSender::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    assert(mController);
    mController->cbAckSetSourceSoundProperties(handle, error);
}

void CAmControlSender::setControllerReady()
{
    assert(mController);
    mController->setControllerReady();
}

void CAmControlSender::setControllerRundown()
{
    assert(mController);
    mController->setControllerRundown();
}

am_Error_e am::CAmControlSender::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e> & listPrioConnectionFormats)
{
    assert(mController);
    return (mController->getConnectionFormatChoice(sourceID, sinkID, listRoute, listPossibleConnectionFormats, listPrioConnectionFormats));
}

void CAmControlSender::getInterfaceVersion(std::string & version) const
{
    version = ControlSendVersion;
}

void CAmControlSender::confirmCommandReady()
{
    assert(mController);
    mController->confirmCommandReady();
}

void CAmControlSender::confirmRoutingReady()
{
    assert(mController);
    mController->confirmRoutingReady();
}

void CAmControlSender::confirmCommandRundown()
{
    assert(mController);
    mController->confirmCommandRundown();
}

void CAmControlSender::confirmRoutingRundown()
{
    assert(mController);
    mController->confirmRoutingRundown();
}
}
