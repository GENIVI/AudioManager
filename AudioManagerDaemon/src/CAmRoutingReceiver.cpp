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
 * \file CAmRoutingReceiver.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmRoutingReceiver.h"
#include <cassert>
#include <algorithm>
#include "CAmDatabaseHandler.h"
#include "CAmRoutingSender.h"
#include "CAmControlSender.h"
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSocketHandler.h"

namespace am
{

CAmRoutingReceiver::CAmRoutingReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler) :
        mpDatabaseHandler(iDatabaseHandler), //
        mpRoutingSender(iRoutingSender), //
        mpControlSender(iControlSender), //
        mpSocketHandler(iSocketHandler), //
        mpDBusWrapper(NULL), //
        mListStartupHandles(), //
        mListRundownHandles(), //
        handleCount(0), //
        mWaitStartup(false), //
        mWaitRundown(false)
{
    assert(mpDatabaseHandler!=NULL);
    assert(mpRoutingSender!=NULL);
    assert(mpControlSender!=NULL);
    assert(mpSocketHandler!=NULL);
}

CAmRoutingReceiver::CAmRoutingReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler, CAmDbusWrapper *iDBusWrapper) :
        mpDatabaseHandler(iDatabaseHandler), //
        mpRoutingSender(iRoutingSender), //
        mpControlSender(iControlSender), //
        mpSocketHandler(iSocketHandler), //
        mpDBusWrapper(iDBusWrapper), //
        mListStartupHandles(), //
        mListRundownHandles(), //
        handleCount(0), //
        mWaitStartup(false), //
        mWaitRundown(false)
{
    assert(mpDatabaseHandler!=NULL);
    assert(mpRoutingSender!=NULL);
    assert(mpControlSender!=NULL);
    assert(mpSocketHandler!=NULL);
    assert(mpDBusWrapper!=NULL);
}

CAmRoutingReceiver::~CAmRoutingReceiver()
{
}

void CAmRoutingReceiver::ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    mpRoutingSender->removeHandle(handle);
    if (error == E_OK)
    {
        mpDatabaseHandler->changeConnectionFinal(connectionID);
    }
    else
    {
        mpDatabaseHandler->removeConnection(connectionID);
    }
    mpControlSender->cbAckConnect(handle, error);
}

void CAmRoutingReceiver::ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    mpRoutingSender->removeHandle(handle);
    if (error == E_OK)
    {
        mpDatabaseHandler->removeConnection(connectionID);
    }
    mpControlSender->cbAckDisconnect(handle, error);
}

void CAmRoutingReceiver::ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sinkID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mpDatabaseHandler->changeSinkVolume(handleData.sinkID, volume);
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSinkVolumeChange(handle, volume, error);
}

void CAmRoutingReceiver::ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mpDatabaseHandler->changeSourceVolume(handleData.sourceID, volume);
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSourceVolumeChange(handle, volume, error);
}

void CAmRoutingReceiver::ackSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mpDatabaseHandler->changeSourceState(handleData.sourceID, handleData.sourceState);
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSourceState(handle, error);
}

void CAmRoutingReceiver::ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sinkID != 0)
    {
        mpDatabaseHandler->changeSinkSoundPropertyDB(handleData.soundPropery, handleData.sinkID);
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSinkSoundProperty(handle, error);

}

void am::CAmRoutingReceiver::ackSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sinkID != 0)
    {
        std::vector<am_SoundProperty_s>::const_iterator it = handleData.soundProperties->begin();
        for (; it != handleData.soundProperties->end(); ++it)
        {
            mpDatabaseHandler->changeSinkSoundPropertyDB(*it, handleData.sinkID);
        }
        delete handleData.soundProperties;
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSinkSoundProperties(handle, error);
}

void CAmRoutingReceiver::ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        mpDatabaseHandler->changeSourceSoundPropertyDB(handleData.soundPropery, handleData.sourceID);
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSourceSoundProperty(handle, error);
}

void am::CAmRoutingReceiver::ackSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        std::vector<am_SoundProperty_s>::const_iterator it = handleData.soundProperties->begin();
        for (; it != handleData.soundProperties->end(); ++it)
        {
            mpDatabaseHandler->changeSourceSoundPropertyDB(*it, handleData.sourceID);
        }
        delete handleData.soundProperties;
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckSetSourceSoundProperties(handle, error);
}

void CAmRoutingReceiver::ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mpRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.crossfaderID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mpDatabaseHandler->changeCrossFaderHotSink(handleData.crossfaderID, hotSink);
    }
    mpRoutingSender->removeHandle(handle);
    mpControlSender->cbAckCrossFade(handle, hotSink, error);
}

void CAmRoutingReceiver::ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    mpControlSender->hookSystemSourceVolumeTick(handle, sourceID, volume);
}

void CAmRoutingReceiver::ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    mpControlSender->hookSystemSinkVolumeTick(handle, sinkID, volume);
}

am_Error_e CAmRoutingReceiver::peekDomain(const std::string & name, am_domainID_t & domainID)
{
    return (mpDatabaseHandler->peekDomain(name, domainID));

}

am_Error_e CAmRoutingReceiver::registerDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    return (mpControlSender->hookSystemRegisterDomain(domainData, domainID));
}

am_Error_e CAmRoutingReceiver::deregisterDomain(const am_domainID_t domainID)
{
    return (mpControlSender->hookSystemDeregisterDomain(domainID));
}

am_Error_e CAmRoutingReceiver::registerGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    return (mpControlSender->hookSystemRegisterGateway(gatewayData, gatewayID));
}

am_Error_e CAmRoutingReceiver::deregisterGateway(const am_gatewayID_t gatewayID)
{
    return (mpControlSender->hookSystemDeregisterGateway(gatewayID));
}

am_Error_e CAmRoutingReceiver::peekSink(const std::string& name, am_sinkID_t & sinkID)
{
    return (mpDatabaseHandler->peekSink(name, sinkID));
}

am_Error_e CAmRoutingReceiver::registerSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    return (mpControlSender->hookSystemRegisterSink(sinkData, sinkID));
}

am_Error_e CAmRoutingReceiver::deregisterSink(const am_sinkID_t sinkID)
{
    return (mpControlSender->hookSystemDeregisterSink(sinkID));
}

am_Error_e CAmRoutingReceiver::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
    return (mpDatabaseHandler->peekSource(name, sourceID));
}

am_Error_e CAmRoutingReceiver::registerSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    return (mpControlSender->hookSystemRegisterSource(sourceData, sourceID));
}

am_Error_e CAmRoutingReceiver::deregisterSource(const am_sourceID_t sourceID)
{
    return (mpControlSender->hookSystemDeregisterSource(sourceID));
}

am_Error_e CAmRoutingReceiver::registerCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    return (mpControlSender->hookSystemRegisterCrossfader(crossfaderData, crossfaderID));
}

am_Error_e CAmRoutingReceiver::deregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    return (mpControlSender->hookSystemDeregisterCrossfader(crossfaderID));
}

void CAmRoutingReceiver::hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    return (mpControlSender->hookSystemInterruptStateChange(sourceID, interruptState));
}

void CAmRoutingReceiver::hookDomainRegistrationComplete(const am_domainID_t domainID)
{
    mpControlSender->hookSystemDomainRegistrationComplete(domainID);
}

void CAmRoutingReceiver::hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mpControlSender->hookSystemSinkAvailablityStateChange(sinkID, availability);
}

void CAmRoutingReceiver::hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mpControlSender->hookSystemSourceAvailablityStateChange(sourceID, availability);
}

void CAmRoutingReceiver::hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    mpControlSender->hookSystemDomainStateChange(domainID, domainState);
}

void CAmRoutingReceiver::hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
    mpDatabaseHandler->changeConnectionTimingInformation(connectionID, delay);
}

void CAmRoutingReceiver::sendChangedData(const std::vector<am_EarlyData_s> & earlyData)
{
    mpControlSender->hookSystemReceiveEarlyData(earlyData);
}

am_Error_e CAmRoutingReceiver::peekSinkClassID(const std::string& name, am_sinkClass_t& sinkClassID)
{
    return (mpDatabaseHandler->peekSinkClassID(name, sinkClassID));
}

am_Error_e CAmRoutingReceiver::peekSourceClassID(const std::string& name, am_sourceClass_t& sourceClassID)
{
    return (mpDatabaseHandler->peekSourceClassID(name, sourceClassID));
}

am_Error_e CAmRoutingReceiver::getDBusConnectionWrapper(CAmDbusWrapper *& dbusConnectionWrapper) const
{
#ifdef WITH_DBUS_WRAPPER
    dbusConnectionWrapper = mpDBusWrapper;
    return (E_OK);
#else
    return (E_UNKNOWN);
#endif
}

am_Error_e CAmRoutingReceiver::getSocketHandler(CAmSocketHandler *& socketHandler) const
{
    socketHandler = mpSocketHandler;
    return (E_OK);
}

void CAmRoutingReceiver::getInterfaceVersion(std::string & version) const
{
    version = RoutingReceiveVersion;
}

void CAmRoutingReceiver::confirmRoutingReady(const uint16_t handle)
{
    mListStartupHandles.erase(std::remove(mListStartupHandles.begin(), mListStartupHandles.end(), handle), mListStartupHandles.end());
    if (mWaitStartup && mListStartupHandles.empty())
        mpControlSender->confirmRoutingReady();
}

void CAmRoutingReceiver::confirmRoutingRundown(const uint16_t handle)
{
    mListRundownHandles.erase(std::remove(mListRundownHandles.begin(), mListRundownHandles.end(), handle), mListRundownHandles.end());
    if (mWaitRundown && mListRundownHandles.empty())
        mpControlSender->confirmRoutingRundown();
}

uint16_t am::CAmRoutingReceiver::getStartupHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListStartupHandles.push_back(handle);
    return (handle);
}

uint16_t am::CAmRoutingReceiver::getRundownHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListRundownHandles.push_back(handle);
    return (handle);
}

void am::CAmRoutingReceiver::waitOnStartup(bool startup)
{
    mWaitStartup = startup;
}

void am::CAmRoutingReceiver::waitOnRundown(bool rundown)
{
    mWaitRundown = rundown;
}
}
