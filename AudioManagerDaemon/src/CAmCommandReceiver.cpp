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
 * \file CAmCommandReceiver.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmCommandReceiver.h"
#include <cassert>
#include <algorithm>
#include "CAmDatabaseHandler.h"
#include "CAmControlSender.h"
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSocketHandler.h"

namespace am
{

CAmCommandReceiver::CAmCommandReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler) :
        mDatabaseHandler(iDatabaseHandler), //
        mControlSender(iControlSender), //
        mDBusWrapper(NULL), //
        mSocketHandler(iSocketHandler), //
        handleCount(0),//
        mListStartupHandles(), //
        mListRundownHandles(), //
        mWaitStartup(false), //
        mWaitRundown(false)

{
    assert(mDatabaseHandler!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mControlSender!=NULL);
}

CAmCommandReceiver::CAmCommandReceiver(CAmDatabaseHandler *iDatabaseHandler, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler, CAmDbusWrapper *iDBusWrapper) :
        mDatabaseHandler(iDatabaseHandler), //
        mControlSender(iControlSender), //
        mDBusWrapper(iDBusWrapper), //
        mSocketHandler(iSocketHandler), //
        handleCount(0),//
        mListStartupHandles(), //
        mListRundownHandles(), //
        mWaitStartup(false), //
        mWaitRundown(false)
{
    assert(mDatabaseHandler!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mControlSender!=NULL);
    assert(mDBusWrapper!=NULL);
}

CAmCommandReceiver::~CAmCommandReceiver()
{
}

am_Error_e CAmCommandReceiver::connect(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    logInfo("CommandReceiver::connect got called, sourceID=", sourceID, "sinkID=", sinkID);
    return (mControlSender->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));
}

am_Error_e CAmCommandReceiver::disconnect(const am_mainConnectionID_t mainConnectionID)
{
    logInfo("CommandReceiver::disconnect got called, mainConnectionID=", mainConnectionID);
    return (mControlSender->hookUserDisconnectionRequest(mainConnectionID));
}

am_Error_e CAmCommandReceiver::setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    logInfo("CommandReceiver::setVolume got called, sinkID=", sinkID, "volume=", volume);
    return (mControlSender->hookUserVolumeChange(sinkID, volume));
}

am_Error_e CAmCommandReceiver::volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep)
{
    logInfo("CommandReceiver::volumeStep got called, sinkID=", sinkID, "volumeStep=", volumeStep);
    return (mControlSender->hookUserVolumeStep(sinkID, volumeStep));
}

am_Error_e CAmCommandReceiver::setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    logInfo("CommandReceiver::setSinkMuteState got called, sinkID=", sinkID, "muteState=", muteState);
    return (mControlSender->hookUserSetSinkMuteState(sinkID, muteState));
}

am_Error_e CAmCommandReceiver::setMainSinkSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    logInfo("CommandReceiver::setMainSinkSoundProperty got called, sinkID=", sinkID, "soundPropertyType=", soundProperty.type, "soundPropertyValue=", soundProperty.value);
    return (mControlSender->hookUserSetMainSinkSoundProperty(sinkID, soundProperty));
}

am_Error_e CAmCommandReceiver::setMainSourceSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    logInfo("CommandReceiver::setMainSourceSoundProperty got called, sourceID=", sourceID, "soundPropertyType=", soundProperty.type, "soundPropertyValue=", soundProperty.value);
    return (mControlSender->hookUserSetMainSourceSoundProperty(sourceID, soundProperty));
}

am_Error_e CAmCommandReceiver::setSystemProperty(const am_SystemProperty_s & property)
{
    logInfo("CommandReceiver::setSystemProperty got called", "type=", property.type, "soundPropertyValue=", property.value);
    return (mControlSender->hookUserSetSystemProperty(property));
}

am_Error_e CAmCommandReceiver::getListMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
    return (mDatabaseHandler->getListVisibleMainConnections(listConnections));

}

am_Error_e CAmCommandReceiver::getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const
{
    return (mDatabaseHandler->getListMainSinks(listMainSinks));
}

am_Error_e CAmCommandReceiver::getListMainSources(std::vector<am_SourceType_s>& listMainSources) const
{
    return (mDatabaseHandler->getListMainSources(listMainSources));
}

am_Error_e CAmCommandReceiver::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
    return (mDatabaseHandler->getListMainSinkSoundProperties(sinkID, listSoundProperties));
}

am_Error_e CAmCommandReceiver::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
    return (mDatabaseHandler->getListMainSourceSoundProperties(sourceID, listSourceProperties));
}

am_Error_e CAmCommandReceiver::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
    return (mDatabaseHandler->getListSourceClasses(listSourceClasses));
}

am_Error_e CAmCommandReceiver::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
    return (mDatabaseHandler->getListSinkClasses(listSinkClasses));
}

am_Error_e CAmCommandReceiver::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
    return (mDatabaseHandler->getListSystemProperties(listSystemProperties));
}

am_Error_e CAmCommandReceiver::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
    return (mDatabaseHandler->getTimingInformation(mainConnectionID, delay));
}

am_Error_e CAmCommandReceiver::getDBusConnectionWrapper(CAmDbusWrapper*& dbusConnectionWrapper) const
{
#ifdef WITH_DBUS_WRAPPER
    dbusConnectionWrapper = mDBusWrapper;
    return (E_OK);
#else
    dbusConnectionWrapper = NULL;
    return (E_UNKNOWN);
#endif /*WITH_DBUS_WRAPPER*/
}

am_Error_e CAmCommandReceiver::getSocketHandler(CAmSocketHandler *& socketHandler) const
{
    socketHandler = mSocketHandler;
    return (E_OK);
}

void CAmCommandReceiver::getInterfaceVersion(std::string & version) const
{
    version = CommandReceiveVersion;
}

void CAmCommandReceiver::confirmCommandReady(const uint16_t handle)
{
    mListStartupHandles.erase(std::remove(mListStartupHandles.begin(), mListStartupHandles.end(), handle), mListStartupHandles.end());
    if (mWaitStartup && mListStartupHandles.empty())
        mControlSender->confirmCommandReady();
}

void CAmCommandReceiver::confirmCommandRundown(const uint16_t handle)
{
    mListRundownHandles.erase(std::remove(mListRundownHandles.begin(), mListRundownHandles.end(), handle), mListRundownHandles.end());
    if (mWaitRundown && mListRundownHandles.empty())
        mControlSender->confirmCommandRundown();
}

uint16_t CAmCommandReceiver::getStartupHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListStartupHandles.push_back(handle);
    return (handle);
}

uint16_t CAmCommandReceiver::getRundownHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListRundownHandles.push_back(handle);
    return (handle);
}

void CAmCommandReceiver::waitOnStartup(bool startup)
{
    mWaitStartup = startup;
}

void CAmCommandReceiver::waitOnRundown(bool rundown)
{
    mWaitRundown = rundown;
}

}
