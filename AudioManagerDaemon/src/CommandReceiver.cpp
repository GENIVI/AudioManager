/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file CommandReveiver.cpp
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

#include "CommandReceiver.h"
#include "DatabaseHandler.h"
#include "ControlSender.h"
#include "DLTWrapper.h"
#include <SocketHandler.h>
#include <cassert>
#include <algorithm>

using namespace am;

CommandReceiver::CommandReceiver(DatabaseHandler *iDatabaseHandler, ControlSender *iControlSender, SocketHandler *iSocketHandler) :
        mDatabaseHandler(iDatabaseHandler), //
        mControlSender(iControlSender), //
        mSocketHandler(iSocketHandler), //
        mListStartupHandles(), //
        mListRundownHandles(), //
        mWaitStartup(false), //
        mWaitRundown(false)

{
    assert(mDatabaseHandler!=NULL);
    assert(mSocketHandler!=NULL);
    assert(mControlSender!=NULL);
}

CommandReceiver::CommandReceiver(DatabaseHandler *iDatabaseHandler, ControlSender *iControlSender, SocketHandler *iSocketHandler, DBusWrapper *iDBusWrapper) :
        mDatabaseHandler(iDatabaseHandler), //
        mControlSender(iControlSender), //
        mDBusWrapper(iDBusWrapper), //
        mSocketHandler(iSocketHandler), //
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

CommandReceiver::~CommandReceiver()
{
}

am_Error_e CommandReceiver::connect(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    logInfo("CommandReceiver::connect got called, sourceID=", sourceID, "sinkID=", sinkID);
    return mControlSender->hookUserConnectionRequest(sourceID, sinkID, mainConnectionID);
}

am_Error_e CommandReceiver::disconnect(const am_mainConnectionID_t mainConnectionID)
{
    logInfo("CommandReceiver::disconnect got called, mainConnectionID=", mainConnectionID);
    return mControlSender->hookUserDisconnectionRequest(mainConnectionID);
}

am_Error_e CommandReceiver::setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    logInfo("CommandReceiver::setVolume got called, sinkID=", sinkID, "volume=", volume);
    return mControlSender->hookUserVolumeChange(sinkID, volume);
}

am_Error_e CommandReceiver::volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep)
{
    logInfo("CommandReceiver::volumeStep got called, sinkID=", sinkID, "volumeStep=", volumeStep);
    return mControlSender->hookUserVolumeStep(sinkID, volumeStep);
}

am_Error_e CommandReceiver::setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    logInfo("CommandReceiver::setSinkMuteState got called, sinkID=", sinkID, "muteState=", muteState);
    return mControlSender->hookUserSetSinkMuteState(sinkID, muteState);
}

am_Error_e CommandReceiver::setMainSinkSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    logInfo("CommandReceiver::setMainSinkSoundProperty got called, sinkID=", sinkID, "soundPropertyType=", soundProperty.type, "soundPropertyValue=", soundProperty.value);
    return mControlSender->hookUserSetMainSinkSoundProperty(sinkID, soundProperty);
}

am_Error_e CommandReceiver::setMainSourceSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    logInfo("CommandReceiver::setMainSourceSoundProperty got called, sourceID=", sourceID, "soundPropertyType=", soundProperty.type, "soundPropertyValue=", soundProperty.value);
    return mControlSender->hookUserSetMainSourceSoundProperty(sourceID, soundProperty);
}

am_Error_e CommandReceiver::setSystemProperty(const am_SystemProperty_s & property)
{
    logInfo("CommandReceiver::setSystemProperty got called", "type=", property.type, "soundPropertyValue=", property.value);
    return mControlSender->hookUserSetSystemProperty(property);
}

am_Error_e CommandReceiver::getListMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
    return mDatabaseHandler->getListVisibleMainConnections(listConnections);

}

am_Error_e CommandReceiver::getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const
{
    return mDatabaseHandler->getListMainSinks(listMainSinks);
}

am_Error_e CommandReceiver::getListMainSources(std::vector<am_SourceType_s>& listMainSources) const
{
    return mDatabaseHandler->getListMainSources(listMainSources);
}

am_Error_e CommandReceiver::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
    return mDatabaseHandler->getListMainSinkSoundProperties(sinkID, listSoundProperties);
}

am_Error_e CommandReceiver::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
    return mDatabaseHandler->getListMainSourceSoundProperties(sourceID, listSourceProperties);
}

am_Error_e CommandReceiver::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
    return mDatabaseHandler->getListSourceClasses(listSourceClasses);
}

am_Error_e CommandReceiver::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
    return mDatabaseHandler->getListSinkClasses(listSinkClasses);
}

am_Error_e CommandReceiver::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
    return mDatabaseHandler->getListSystemProperties(listSystemProperties);
}

am_Error_e CommandReceiver::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
    return mDatabaseHandler->getTimingInformation(mainConnectionID, delay);
}

am_Error_e CommandReceiver::getDBusConnectionWrapper(DBusWrapper*& dbusConnectionWrapper) const
{
#ifdef WITH_DBUS_WRAPPER
    dbusConnectionWrapper = mDBusWrapper;
    return E_OK;
#else
    return E_UNKNOWN;
#endif /*WITH_DBUS_WRAPPER*/
}

am_Error_e CommandReceiver::getSocketHandler(SocketHandler *& socketHandler) const
{
    socketHandler = mSocketHandler;
    return E_OK;
}

void CommandReceiver::getInterfaceVersion(std::string & version) const
{
    version = CommandReceiveVersion;
}

void CommandReceiver::confirmCommandReady(const uint16_t handle)
{
    mListStartupHandles.erase(std::remove(mListStartupHandles.begin(), mListStartupHandles.end(), handle), mListStartupHandles.end());
    if (mWaitStartup && mListStartupHandles.empty())
        mControlSender->confirmCommandReady();
}

void CommandReceiver::confirmCommandRundown(const uint16_t handle)
{
    mListRundownHandles.erase(std::remove(mListRundownHandles.begin(), mListRundownHandles.end(), handle), mListRundownHandles.end());
    if (mWaitRundown && mListRundownHandles.empty())
        mControlSender->confirmCommandRundown();
}

uint16_t CommandReceiver::getStartupHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListStartupHandles.push_back(handle);
    return handle;
}

uint16_t CommandReceiver::getRundownHandle()
{
    uint16_t handle = ++handleCount; //todo: handle overflow
    mListRundownHandles.push_back(handle);
    return handle;
}

void CommandReceiver::waitOnStartup(bool startup)
{
    mWaitStartup = startup;
}

void CommandReceiver::waitOnRundown(bool rundown)
{
    mWaitRundown = rundown;
}

