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
 * \file CAmCommandSender.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmCommandSender.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include "CAmCommandReceiver.h"
#include "TAmPluginTemplate.h"
#include "CAmDltWrapper.h"
#include "audiomanagerconfig.h"

#define __METHOD_NAME__ std::string(std::string("CAmCommandSender::") + __func__)

namespace am
{

/**
 *  macro to call all interfaces
 */
#define CALL_ALL_INTERFACES(...)                                             \
    std::vector<IAmCommandSend *>::iterator iter = mListInterfaces.begin();  \
    std::vector<IAmCommandSend *>::iterator iterEnd = mListInterfaces.end(); \
    for (; iter < iterEnd; ++iter)                                           \
    {                                                                        \
        (*iter)->__VA_ARGS__;                                                \
    }

CAmCommandSender::CAmCommandSender(const std::vector<std::string> &listOfPluginDirectories, CAmSocketHandler *iSocketHandler)
    : CAmDatabaseHandlerMap::AmDatabaseObserverCallbacks()
    , mListInterfaces()
    , mListLibraryHandles()
    , mListLibraryNames()
    , mCommandReceiver()
    , mSerializer(iSocketHandler)
{
    loadPlugins(listOfPluginDirectories);

    dboNewMainConnection = [&](const am_MainConnectionType_s &mainConnection) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbNewMainConnection, mainConnection);
        };
    dboRemovedMainConnection = [&](const am_mainConnectionID_t mainConnection) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbRemovedMainConnection, mainConnection);
        };
    dboNewSink = [&](const am_Sink_s &sink) {
            if (sink.visible)
            {
                am_SinkType_s s;
                s.availability = sink.available;
                s.muteState    = sink.muteState;
                s.name         = sink.name;
                s.sinkClassID  = sink.sinkClassID;
                s.sinkID       = sink.sinkID;
                s.volume       = sink.mainVolume;
                typedef void (CAmCommandSender::*TMeth)(am::am_SinkType_s);
                mSerializer.asyncCall<CAmCommandSender, TMeth, am::am_SinkType_s>(this, &CAmCommandSender::cbNewSink, s);
            }
        };
    dboNewSource = [&](const am_Source_s &source) {
            if (source.visible)
            {
                am_SourceType_s s;
                s.availability  = source.available;
                s.name          = source.name;
                s.sourceClassID = source.sourceClassID;
                s.sourceID      = source.sourceID;
                typedef void (CAmCommandSender::*TMeth)(am::am_SourceType_s);
                mSerializer.asyncCall<CAmCommandSender, TMeth, am::am_SourceType_s>(this, &CAmCommandSender::cbNewSource, s);
            }
        };

    dboRemovedSink = [&](const am_sinkID_t sinkID, const bool visible) {
            if (visible)
            {
                mSerializer.asyncCall(this, &CAmCommandSender::cbRemovedSink, sinkID);
            }
        };
    dboRemovedSource = [&](const am_sourceID_t sourceID, const bool visible) {
            if (visible)
            {
                mSerializer.asyncCall(this, &CAmCommandSender::cbRemovedSource, sourceID);
            }
        };
    dboNumberOfSinkClassesChanged = [&]() {
            mSerializer.asyncCall(this, &CAmCommandSender::cbNumberOfSinkClassesChanged);
        };
    dboNumberOfSourceClassesChanged = [&]() {
            mSerializer.asyncCall(this, &CAmCommandSender::cbNumberOfSourceClassesChanged);
        };
    dboMainConnectionStateChanged = [&](const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbMainConnectionStateChanged, connectionID, connectionState);
        };
    dboMainSinkSoundPropertyChanged = [&](const am_sinkID_t sinkID, const am_MainSoundProperty_s &SoundProperty) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbMainSinkSoundPropertyChanged, sinkID, SoundProperty);
        };
    dboMainSourceSoundPropertyChanged = [&](const am_sourceID_t sourceID, const am_MainSoundProperty_s &SoundProperty) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbMainSourceSoundPropertyChanged, sourceID, SoundProperty);
        };
    dboSinkAvailabilityChanged = [&](const am_sinkID_t sinkID, const am_Availability_s &availability) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbSinkAvailabilityChanged, sinkID, availability);
        };
    dboSourceAvailabilityChanged = [&](const am_sourceID_t sourceID, const am_Availability_s &availability) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbSourceAvailabilityChanged, sourceID, availability);
        };
    dboVolumeChanged = [&](const am_sinkID_t sinkID, const am_mainVolume_t volume) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbVolumeChanged, sinkID, volume);
        };
    dboSinkMuteStateChanged = [&](const am_sinkID_t sinkID, const am_MuteState_e muteState) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbSinkMuteStateChanged, sinkID, muteState);
        };
	dboSourceMuteStateChanged = [&](const am_sourceID_t sourceID, const am_MuteState_e muteState) {
        mSerializer.asyncCall(this, &CAmCommandSender::cbSourceMuteStateChanged, sourceID, muteState);
    };
    dboSystemPropertyChanged = [&](const am_SystemProperty_s &SystemProperty) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbSystemPropertyChanged, SystemProperty);
        };
    dboTimingInformationChanged = [&](const am_mainConnectionID_t mainConnection, const am_timeSync_t time) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbTimingInformationChanged, mainConnection, time);
        };
    dboSinkUpdated = [&](const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s> &listMainSoundProperties, const bool visible) {
            if (visible)
            {
                mSerializer.asyncCall(this, &CAmCommandSender::cbSinkUpdated, sinkID, sinkClassID, listMainSoundProperties);
            }
        };
    dboSourceUpdated = [&](const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s> &listMainSoundProperties, const bool visible) {
            if (visible)
            {
                mSerializer.asyncCall(this, &CAmCommandSender::cbSinkUpdated, sourceID, sourceClassID, listMainSoundProperties);
            }
        };
    dboSinkMainNotificationConfigurationChanged = [&](const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbSinkMainNotificationConfigurationChanged, sinkID, mainNotificationConfiguration);
        };
    dboSourceMainNotificationConfigurationChanged = [&](const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration) {
            mSerializer.asyncCall(this, &CAmCommandSender::cbSourceMainNotificationConfigurationChanged, sourceID, mainNotificationConfiguration);
        };
}

void CAmCommandSender::loadPlugins(const std::vector<std::string> &listOfPluginDirectories)
{
    if (listOfPluginDirectories.empty())
    {
        logError(__METHOD_NAME__, "List of commandplugins is empty");
    }

    std::vector<std::string>                 sharedLibraryNameList;
    std::vector<std::string>::const_iterator dirIter    = listOfPluginDirectories.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listOfPluginDirectories.end();

    // search communicator plugins in configured directories
    for (; dirIter < dirIterEnd; ++dirIter)
    {
        const char *directoryName = dirIter->c_str();
        logInfo(__METHOD_NAME__, "Searching for CommandPlugins in", *dirIter);
        DIR *directory = opendir(directoryName);

        if (!directory)
        {
            logError(__METHOD_NAME__, "Error opening directory ", *dirIter);
            continue;
        }

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
            unsigned char entryType = itemInDirectory->d_type;
            std::string   entryName = itemInDirectory->d_name;
            std::string   fullName  = *dirIter + "/" + entryName;

            bool regularFile        = (entryType == DT_REG || entryType == DT_LNK);
            bool sharedLibExtension = ("so" == entryName.substr(entryName.find_last_of(".") + 1));

            // Handle cases where readdir() could not determine the file type
            if (entryType == DT_UNKNOWN)
            {
                struct stat buf;

                if (stat(fullName.c_str(), &buf))
                {
                    logInfo(__METHOD_NAME__, "Failed to stat file: ", entryName, errno);
                    continue;
                }

                regularFile = S_ISREG(buf.st_mode);
            }

            if (regularFile && sharedLibExtension)
            {
                std::string name(directoryName);
                sharedLibraryNameList.push_back(name + "/" + entryName);
            }
        }

        closedir(directory);
    }

    // iterate all communicator plugins and start them
    std::vector<std::string>::iterator iter    = sharedLibraryNameList.begin();
    std::vector<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter < iterEnd; ++iter)
    {
        logInfo(__METHOD_NAME__, "Loading CommandSender plugin", *iter);
        IAmCommandSend *(*createFunc)();
        void           *tempLibHandle = NULL;
        createFunc = getCreateFunction<IAmCommandSend *()>(*iter, tempLibHandle);

        if (!createFunc)
        {
            logInfo(__METHOD_NAME__, "Entry point of CommandPlugin not found", *iter);
            continue;
        }

        IAmCommandSend *commander = createFunc();

        if (!commander)
        {
            logInfo(__METHOD_NAME__, "CommandPlugin initialization failed. Entry Function not callable");
            dlclose(tempLibHandle);
            continue;
        }

        // check libversion
        std::string version, cVersion(CommandVersion);
        commander->getInterfaceVersion(version);
        uint16_t minorVersion, majorVersion, cMinorVersion, cMajorVersion;
        std::istringstream(version.substr(0, 1)) >> majorVersion;
        std::istringstream(version.substr(2, 1)) >> minorVersion;
        std::istringstream(cVersion.substr(0, 1)) >> cMajorVersion;
        std::istringstream(cVersion.substr(2, 1)) >> cMinorVersion;

        if (majorVersion < cMajorVersion || ((majorVersion == cMajorVersion) && (minorVersion > cMinorVersion)))
        {
            logError(__METHOD_NAME__, "CommandInterface initialization failed. Version of Interface to old");
            dlclose(tempLibHandle);
            continue;
        }

        mListInterfaces.push_back(commander);
        mListLibraryHandles.push_back(tempLibHandle);
        mListLibraryNames.push_back(iter->c_str());
    }
}

CAmCommandSender::~CAmCommandSender()
{
    // unloadLibraries();
}

am_Error_e CAmCommandSender::startupInterfaces(CAmCommandReceiver *iCommandReceiver)
{
    mCommandReceiver = iCommandReceiver;
    am_Error_e returnError = E_OK;

    std::vector<IAmCommandSend *>::iterator iter    = mListInterfaces.begin();
    std::vector<IAmCommandSend *>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        am_Error_e error = (*iter)->startupInterface(iCommandReceiver);
        if (error != E_OK)
        {
            returnError = error;
        }
    }

    return (returnError);
}

void CAmCommandSender::cbNumberOfSinkClassesChanged()
{
    CALL_ALL_INTERFACES(cbNumberOfSinkClassesChanged())
}

void CAmCommandSender::cbNumberOfSourceClassesChanged()
{
    CALL_ALL_INTERFACES(cbNumberOfSourceClassesChanged())
}

void CAmCommandSender::cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
    CALL_ALL_INTERFACES(cbMainConnectionStateChanged(connectionID, connectionState))
}

void CAmCommandSender::cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s &SoundProperty)
{
    CALL_ALL_INTERFACES(cbMainSinkSoundPropertyChanged(sinkID, SoundProperty))
}

void CAmCommandSender::cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s &SoundProperty)
{
    CALL_ALL_INTERFACES(cbMainSourceSoundPropertyChanged(sourceID, SoundProperty))
}

void CAmCommandSender::cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s &availability)
{
    CALL_ALL_INTERFACES(cbSinkAvailabilityChanged(sinkID, availability))
}

void CAmCommandSender::cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s &availability)
{
    CALL_ALL_INTERFACES(cbSourceAvailabilityChanged(sourceID, availability))
}

void CAmCommandSender::cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    CALL_ALL_INTERFACES(cbVolumeChanged(sinkID, volume))
}

void CAmCommandSender::cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    CALL_ALL_INTERFACES(cbSinkMuteStateChanged(sinkID, muteState))
}

void CAmCommandSender::cbSourceMuteStateChanged(const am_sourceID_t sourceID, const am_MuteState_e muteState)
{
    CALL_ALL_INTERFACES(cbSourceMuteStateChanged(sourceID,muteState))
}

void CAmCommandSender::cbSystemPropertyChanged(const am_SystemProperty_s &SystemProperty)
{
    CALL_ALL_INTERFACES(cbSystemPropertyChanged(SystemProperty))
}

void CAmCommandSender::cbTimingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
    CALL_ALL_INTERFACES(cbTimingInformationChanged(mainConnection, time))
}

void CAmCommandSender::cbNewMainConnection(const am_MainConnectionType_s mainConnection)
{
    CALL_ALL_INTERFACES(cbNewMainConnection(mainConnection))
}

void CAmCommandSender::cbRemovedMainConnection(const am_mainConnectionID_t mainConnection)
{
    CALL_ALL_INTERFACES(cbRemovedMainConnection(mainConnection))
}

void CAmCommandSender::cbNewSink(const am_SinkType_s sink)
{
    CALL_ALL_INTERFACES(cbNewSink(sink))
}

void CAmCommandSender::cbRemovedSink(const am_sinkID_t sink)
{
    CALL_ALL_INTERFACES(cbRemovedSink(sink))
}

void CAmCommandSender::cbNewSource(const am_SourceType_s source)
{
    CALL_ALL_INTERFACES(cbNewSource(source))
}

void CAmCommandSender::cbRemovedSource(const am_sourceID_t source)
{
    CALL_ALL_INTERFACES(cbRemovedSource(source))
}

void CAmCommandSender::setCommandReady()
{
    mCommandReceiver->waitOnStartup(false);

    // create a list of handles
    std::vector<uint16_t> listStartupHandles;
    for (size_t i = 0; i < mListInterfaces.size(); i++)
    {
        listStartupHandles.push_back(mCommandReceiver->getStartupHandle());
    }

    // set the receiver ready to wait for replies
    mCommandReceiver->waitOnStartup(true);

    // now do the calls
    std::vector<IAmCommandSend *>::iterator iter    = mListInterfaces.begin();
    std::vector<IAmCommandSend *>::iterator iterEnd = mListInterfaces.end();
    std::vector<uint16_t>::const_iterator   handleIter(listStartupHandles.begin());
    for (; iter < iterEnd; ++iter)
    {
        (*iter)->setCommandReady(*(handleIter++));
    }
}

void CAmCommandSender::setCommandRundown()
{
    mCommandReceiver->waitOnRundown(false);
    // create a list of handles
    std::vector<uint16_t> listStartupHandles;
    for (size_t i = 0; i < mListInterfaces.size(); i++)
    {
        listStartupHandles.push_back(mCommandReceiver->getRundownHandle());
    }

    // set the receiver ready to wait for replies
    mCommandReceiver->waitOnRundown(true);

    // now do the calls
    std::vector<IAmCommandSend *>::iterator iter    = mListInterfaces.begin();
    std::vector<IAmCommandSend *>::iterator iterEnd = mListInterfaces.end();
    std::vector<uint16_t>::const_iterator   handleIter(listStartupHandles.begin());
    for (; iter < iterEnd; ++iter)
    {
        (*iter)->setCommandRundown(*(handleIter++));
    }
}

void CAmCommandSender::getInterfaceVersion(std::string &version) const
{
    version = CommandVersion;
}

am_Error_e am::CAmCommandSender::getListPlugins(std::vector<std::string> &interfaces) const
{
    interfaces = mListLibraryNames;
    return (E_OK);
}

void CAmCommandSender::cbSinkUpdated(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s> &listMainSoundProperties)
{
    CALL_ALL_INTERFACES(cbSinkUpdated(sinkID, sinkClassID, listMainSoundProperties));
}

void CAmCommandSender::cbSourceUpdated(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s> &listMainSoundProperties)
{
    CALL_ALL_INTERFACES(cbSourceUpdated(sourceID, sourceClassID, listMainSoundProperties));
}

void CAmCommandSender::cbSinkNotification(const am_sinkID_t sinkID, const am_NotificationPayload_s &notification)
{
    CALL_ALL_INTERFACES(cbSinkNotification(sinkID, notification));
}

void CAmCommandSender::cbSourceNotification(const am_sourceID_t sourceID, const am_NotificationPayload_s &notification)
{
    CALL_ALL_INTERFACES(cbSourceNotification(sourceID, notification));
}

void CAmCommandSender::cbSinkMainNotificationConfigurationChanged(const am_sinkID_t sinkID, const am_NotificationConfiguration_s &mainNotificationConfiguration)
{
    CALL_ALL_INTERFACES(cbMainSinkNotificationConfigurationChanged(sinkID, mainNotificationConfiguration));
}

void CAmCommandSender::cbSourceMainNotificationConfigurationChanged(const am_sourceID_t sourceID, const am_NotificationConfiguration_s &mainNotificationConfiguration)
{
    CALL_ALL_INTERFACES(cbMainSourceNotificationConfigurationChanged(sourceID, mainNotificationConfiguration));
}

void CAmCommandSender::unloadLibraries(void)
{
    std::vector<void *>::iterator iterator = mListLibraryHandles.begin();
    for (; iterator < mListLibraryHandles.end(); ++iterator)
    {
        dlclose(*iterator);
    }

    mListLibraryHandles.clear();
}

}
