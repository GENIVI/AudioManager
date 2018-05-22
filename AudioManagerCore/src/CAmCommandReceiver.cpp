/**
 * SPDX license identifier: MPL-2.0
 *
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
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * \file CAmCommandReceiver.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmCommandReceiver.h"
#include <cassert>
#include <algorithm>
#include "IAmDatabaseHandler.h"
#include "CAmControlSender.h"
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"

#define __METHOD_NAME__ std::string (std::string("CAmCommandReceiver::") + __func__)

namespace am
{

CAmCommandReceiver::CAmCommandReceiver(IAmDatabaseHandler *iDatabaseHandler, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler) :
        mDatabaseHandler(iDatabaseHandler),
        mControlSender(iControlSender),
        mDBusWrapper(NULL),
        mSocketHandler(iSocketHandler),
        handleCount(0),
        mListStartupHandles(),
        mListRundownHandles(),
        mWaitStartup(false),
        mWaitRundown(false),
        mLastErrorStartup(E_OK),
        mLastErrorRundown(E_OK)

{
    assert(mDatabaseHandler!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mControlSender!=NULL);
}

CAmCommandReceiver::CAmCommandReceiver(IAmDatabaseHandler *iDatabaseHandler, CAmControlSender *iControlSender, CAmSocketHandler *iSocketHandler, CAmDbusWrapper *iDBusWrapper) :
        mDatabaseHandler(iDatabaseHandler),
        mControlSender(iControlSender),
        mDBusWrapper(iDBusWrapper),
        mSocketHandler(iSocketHandler),
        handleCount(0),
        mListStartupHandles(),
        mListRundownHandles(),
        mWaitStartup(false),
        mWaitRundown(false),
        mLastErrorStartup(E_UNKNOWN),
        mLastErrorRundown(E_UNKNOWN)
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
    logInfo(__METHOD_NAME__,"sourceID=", sourceID, "sinkID=", sinkID);
    return (mControlSender->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID));
}

am_Error_e CAmCommandReceiver::disconnect(const am_mainConnectionID_t mainConnectionID)
{
    logInfo(__METHOD_NAME__,"mainConnectionID=", mainConnectionID);
    return (mControlSender->hookUserDisconnectionRequest(mainConnectionID));
}

am_Error_e CAmCommandReceiver::setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, "volume=", volume);
    return (mControlSender->hookUserVolumeChange(sinkID, volume));
}

am_Error_e CAmCommandReceiver::volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep)
{
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, "volumeStep=", volumeStep);
    return (mControlSender->hookUserVolumeStep(sinkID, volumeStep));
}

am_Error_e CAmCommandReceiver::setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, "muteState=", muteState);
    return (mControlSender->hookUserSetSinkMuteState(sinkID, muteState));
}

am_Error_e CAmCommandReceiver::setMainSinkSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, "soundPropertyType=", soundProperty.type, "soundPropertyValue=", soundProperty.value);
    return (mControlSender->hookUserSetMainSinkSoundProperty(sinkID, soundProperty));
}

am_Error_e CAmCommandReceiver::setMainSourceSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    logInfo(__METHOD_NAME__,"sourceID=", sourceID, "soundPropertyType=", soundProperty.type, "soundPropertyValue=", soundProperty.value);
    return (mControlSender->hookUserSetMainSourceSoundProperty(sourceID, soundProperty));
}

am_Error_e CAmCommandReceiver::setSystemProperty(const am_SystemProperty_s & property)
{
    logInfo(__METHOD_NAME__,"type=", property.type, "systemPropertyValue=", property.value);
    return (mControlSender->hookUserSetSystemProperty(property));
}

am_Error_e CAmCommandReceiver::getVolume(const am_sinkID_t sinkID, am_mainVolume_t& mainVolume) const
{
    return (mDatabaseHandler->getSinkMainVolume(sinkID, mainVolume));
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
    version = CommandVersion;
}

void CAmCommandReceiver::confirmCommandReady(const uint16_t handle, const am_Error_e error)
{
	if (error !=E_OK)
		mLastErrorStartup=error;
    mListStartupHandles.erase(std::remove(mListStartupHandles.begin(), mListStartupHandles.end(), handle), mListStartupHandles.end());
    if (mWaitStartup && mListStartupHandles.empty())
        mControlSender->confirmCommandReady(mLastErrorStartup);
}

void CAmCommandReceiver::confirmCommandRundown(const uint16_t handle, const am_Error_e error)
{
	if (error !=E_OK)
		mLastErrorRundown=error;
    mListRundownHandles.erase(std::remove(mListRundownHandles.begin(), mListRundownHandles.end(), handle), mListRundownHandles.end());
    if (mWaitRundown && mListRundownHandles.empty())
        mControlSender->confirmCommandRundown(mLastErrorRundown);
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
    mLastErrorStartup=E_OK;
}

am_Error_e CAmCommandReceiver::getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations) const
{
    return (mDatabaseHandler->getListMainSinkNotificationConfigurations(sinkID,listMainNotificationConfigurations));
}

am_Error_e CAmCommandReceiver::getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations) const
{
    return (mDatabaseHandler->getListMainSourceNotificationConfigurations(sourceID,listMainNotificationConfigurations));
}

am_Error_e CAmCommandReceiver::setMainSinkNotificationConfiguration(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration)
{
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, " type=",mainNotificationConfiguration.type, " parameter=", mainNotificationConfiguration.parameter, "status=",mainNotificationConfiguration.status);
    return (mControlSender->hookUserSetMainSinkNotificationConfiguration(sinkID,mainNotificationConfiguration));
}

am_Error_e CAmCommandReceiver::setMainSourceNotificationConfiguration(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration)
{
    logInfo(__METHOD_NAME__,"sourceID=", sourceID, " type=",mainNotificationConfiguration.type, " parameter=", mainNotificationConfiguration.parameter, "status=",mainNotificationConfiguration.status);
    return (mControlSender->hookUserSetMainSourceNotificationConfiguration(sourceID,mainNotificationConfiguration));
}

void CAmCommandReceiver::waitOnRundown(bool rundown)
{
    mWaitRundown = rundown;
    mLastErrorStartup=E_OK;
}

}
