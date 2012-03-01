/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file CAmRoutingReceiver.cpp
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section LicenseCAmRoutingReceiver.h
 *
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
        mDatabaseHandler(iDatabaseHandler), //
        mRoutingSender(iRoutingSender), //
        mControlSender(iControlSender), //
        mSocketHandler(iSocketHandler), //
        mListStartupHandles(), //
        mListRundownHandles(), //
        mWaitStartup(false), //
        mWaitRundown(false)
{
    assert(mDatabaseHandler!=NULL);
    assert(mRoutingSender!=NULL);
    assert(mControlSender!=NULL);
    assert(mSocketHandler!=NULL);
}

CAmRoutingReceiver::CAmRoutingReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmRoutingSender *iRoutingSender, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler, CAmDbusWrapper *iDBusWrapper) :
        mDatabaseHandler(iDatabaseHandler), //
        mRoutingSender(iRoutingSender), //
        mControlSender(iControlSender), //
        mSocketHandler(iSocketHandler), //
        mDBusWrapper(iDBusWrapper), //
        mListStartupHandles(), //
        mListRundownHandles(), //
        mWaitStartup(false), //
        mWaitRundown(false)
{
    assert(mDatabaseHandler!=NULL);
    assert(mRoutingSender!=NULL);
    assert(mControlSender!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mDBusWrapper!=NULL);
}

CAmRoutingReceiver::~CAmRoutingReceiver()
{
}

void CAmRoutingReceiver::ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    mRoutingSender->removeHandle(handle);
    if (error == E_OK)
    {
        mDatabaseHandler->changeConnectionFinal(connectionID);
    }
    else
    {
        mDatabaseHandler->removeConnection(connectionID);
    }
    mControlSender->cbAckConnect(handle, error);
}

void CAmRoutingReceiver::ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    mRoutingSender->removeHandle(handle);
    if (error == E_OK)
    {
        mDatabaseHandler->removeConnection(connectionID);
    }
    mControlSender->cbAckDisconnect(handle, error);
}

void CAmRoutingReceiver::ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sinkID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mDatabaseHandler->changeSinkVolume(handleData.sinkID, volume);
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSinkVolumeChange(handle, volume, error);
}

void CAmRoutingReceiver::ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mDatabaseHandler->changeSourceVolume(handleData.sourceID, volume);
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSourceVolumeChange(handle, volume, error);
}

void CAmRoutingReceiver::ackSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mDatabaseHandler->changeSourceState(handleData.sourceID, handleData.sourceState);
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSourceState(handle, error);
}

void CAmRoutingReceiver::ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sinkID != 0)
    {
        mDatabaseHandler->changeSinkSoundPropertyDB(handleData.soundPropery, handleData.sinkID);
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSinkSoundProperty(handle, error);

}

void am::CAmRoutingReceiver::ackSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sinkID != 0)
    {
        std::vector<am_SoundProperty_s>::const_iterator it = handleData.soundProperties->begin();
        for (; it != handleData.soundProperties->end(); ++it)
        {
            mDatabaseHandler->changeSinkSoundPropertyDB(*it, handleData.sinkID);
        }
        delete handleData.soundProperties;
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSinkSoundProperties(handle, error);
}

void CAmRoutingReceiver::ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        mDatabaseHandler->changeSourceSoundPropertyDB(handleData.soundPropery, handleData.sourceID);
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSourceSoundProperty(handle, error);
}

void am::CAmRoutingReceiver::ackSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.sourceID != 0)
    {
        std::vector<am_SoundProperty_s>::const_iterator it = handleData.soundProperties->begin();
        for (; it != handleData.soundProperties->end(); ++it)
        {
            mDatabaseHandler->changeSourceSoundPropertyDB(*it, handleData.sourceID);
        }
        delete handleData.soundProperties;
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckSetSourceSoundProperties(handle, error);
}

void CAmRoutingReceiver::ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error)
{
    CAmRoutingSender::am_handleData_c handleData = mRoutingSender->returnHandleData(handle);
    if (error == E_OK && handleData.crossfaderID != 0)
    {
        //todo: check if volume in handleData is same than volume. React to it.
        mDatabaseHandler->changeCrossFaderHotSink(handleData.crossfaderID, hotSink);
    }
    mRoutingSender->removeHandle(handle);
    mControlSender->cbAckCrossFade(handle, hotSink, error);
}

void CAmRoutingReceiver::ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    mControlSender->hookSystemSourceVolumeTick(handle, sourceID, volume);
}

void CAmRoutingReceiver::ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    mControlSender->hookSystemSinkVolumeTick(handle, sinkID, volume);
}

am_Error_e CAmRoutingReceiver::peekDomain(const std::string & name, am_domainID_t & domainID)
{
    return mDatabaseHandler->peekDomain(name, domainID);

}

am_Error_e CAmRoutingReceiver::registerDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    return mControlSender->hookSystemRegisterDomain(domainData, domainID);
}

am_Error_e CAmRoutingReceiver::deregisterDomain(const am_domainID_t domainID)
{
    return mControlSender->hookSystemDeregisterDomain(domainID);
}

am_Error_e CAmRoutingReceiver::registerGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    return mControlSender->hookSystemRegisterGateway(gatewayData, gatewayID);
}

am_Error_e CAmRoutingReceiver::deregisterGateway(const am_gatewayID_t gatewayID)
{
    return mControlSender->hookSystemDeregisterGateway(gatewayID);
}

am_Error_e CAmRoutingReceiver::peekSink(const std::string& name, am_sinkID_t & sinkID)
{
    return mDatabaseHandler->peekSink(name, sinkID);
}

am_Error_e CAmRoutingReceiver::registerSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    return mControlSender->hookSystemRegisterSink(sinkData, sinkID);
}

am_Error_e CAmRoutingReceiver::deregisterSink(const am_sinkID_t sinkID)
{
    return mControlSender->hookSystemDeregisterSink(sinkID);
}

am_Error_e CAmRoutingReceiver::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
    return mDatabaseHandler->peekSource(name, sourceID);
}

am_Error_e CAmRoutingReceiver::registerSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    return mControlSender->hookSystemRegisterSource(sourceData, sourceID);
}

am_Error_e CAmRoutingReceiver::deregisterSource(const am_sourceID_t sourceID)
{
    return mControlSender->hookSystemDeregisterSource(sourceID);
}

am_Error_e CAmRoutingReceiver::registerCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    return mControlSender->hookSystemRegisterCrossfader(crossfaderData, crossfaderID);
}

am_Error_e CAmRoutingReceiver::deregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    return mControlSender->hookSystemDeregisterCrossfader(crossfaderID);
}

void CAmRoutingReceiver::hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    return mControlSender->hookSystemInterruptStateChange(sourceID, interruptState);
}

void CAmRoutingReceiver::hookDomainRegistrationComplete(const am_domainID_t domainID)
{
    mControlSender->hookSystemDomainRegistrationComplete(domainID);
}

void CAmRoutingReceiver::hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mControlSender->hookSystemSinkAvailablityStateChange(sinkID, availability);
}

void CAmRoutingReceiver::hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mControlSender->hookSystemSourceAvailablityStateChange(sourceID, availability);
}

void CAmRoutingReceiver::hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    mControlSender->hookSystemDomainStateChange(domainID, domainState);
}

void CAmRoutingReceiver::hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
    mDatabaseHandler->changeConnectionTimingInformation(connectionID, delay);
}

void CAmRoutingReceiver::sendChangedData(const std::vector<am_EarlyData_s> & earlyData)
{
    mControlSender->hookSystemReceiveEarlyData(earlyData);
}

am_Error_e CAmRoutingReceiver::peekSinkClassID(const std::string& name, am_sinkClass_t& sinkClassID)
{
    return mDatabaseHandler->peekSinkClassID(name, sinkClassID);
}

am_Error_e CAmRoutingReceiver::peekSourceClassID(const std::string& name, am_sourceClass_t& sourceClassID)
{
    return mDatabaseHandler->peekSourceClassID(name, sourceClassID);
}

am_Error_e CAmRoutingReceiver::getDBusConnectionWrapper(CAmDbusWrapper *& dbusConnectionWrapper) const
{
#ifdef WITH_DBUS_WRAPPER
    dbusConnectionWrapper = mDBusWrapper;
    return E_OK;
#else
    return E_UNKNOWN;
#endif
}

am_Error_e CAmRoutingReceiver::getSocketHandler(CAmSocketHandler *& socketHandler) const
{
    socketHandler = mSocketHandler;
    return E_OK;
}

void CAmRoutingReceiver::getInterfaceVersion(std::string & version) const
{
    version = RoutingReceiveVersion;
}

void CAmRoutingReceiver::confirmRoutingReady(const uint16_t handle)
{
    mListStartupHandles.erase(std::remove(mListStartupHandles.begin(), mListStartupHandles.end(), handle), mListStartupHandles.end());
    if (mWaitStartup && mListStartupHandles.empty())
        mControlSender->confirmRoutingReady();
}

void CAmRoutingReceiver::confirmRoutingRundown(const uint16_t handle)
{
    mListRundownHandles.erase(std::remove(mListRundownHandles.begin(), mListRundownHandles.end(), handle), mListRundownHandles.end());
    if (mWaitRundown && mListRundownHandles.empty())
        mControlSender->confirmCommandRundown();
}

uint16_t am::CAmRoutingReceiver::getStartupHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListStartupHandles.push_back(handle);
    return handle;
}

uint16_t am::CAmRoutingReceiver::getRundownHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListRundownHandles.push_back(handle);
    return handle;
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
