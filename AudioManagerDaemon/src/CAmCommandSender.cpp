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
 * \file CAmCommandSender.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmCommandSender.h"
#include <dirent.h>
#include <sstream>
#include <string>
#include "CAmCommandReceiver.h"
#include "TAmPluginTemplate.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

#define REQUIRED_INTERFACE_VERSION_MAJOR 1  //!< major interface version. All versions smaller than this will be rejected
#define REQUIRED_INTERFACE_VERSION_MINOR 0 //!< minor interface version. All versions smaller than this will be rejected
/**
 *  macro to call all interfaces
 */
#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();	         \
		std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();	         \
		for (; iter<iterEnd;++iter)													 	 \
		{																				 \
			(*iter)->__VA_ARGS__;													 	 \
		}

CAmCommandSender::CAmCommandSender(const std::vector<std::string>& listOfPluginDirectories) :
        mListInterfaces(), //
        mListLibraryHandles(), //
        mListLibraryNames(), //
        mCommandReceiver()
{
    std::vector<std::string> sharedLibraryNameList;
    std::vector<std::string>::const_iterator dirIter = listOfPluginDirectories.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listOfPluginDirectories.end();

    // search communicator plugins in configured directories
    for (; dirIter < dirIterEnd; ++dirIter)
    {
        const char* directoryName = dirIter->c_str();
        logInfo("Searching for CommandPlugins in", *dirIter);
        DIR *directory = opendir(directoryName);

        if (!directory)
        {
            logError("Error opening directory ", *dirIter);
            continue;
        }

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
            unsigned char entryType = itemInDirectory->d_type;
            std::string entryName = itemInDirectory->d_name;

            bool regularFile = (entryType == DT_REG || entryType == DT_LNK);
            bool sharedLibExtension = ("so" == entryName.substr(entryName.find_last_of(".") + 1));

            if (regularFile && sharedLibExtension)
            {
                std::string name(directoryName);
                sharedLibraryNameList.push_back(name + "/" + entryName);
            }
        }
        closedir(directory);
    }

    // iterate all communicator plugins and start them
    std::vector<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::vector<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter < iterEnd; ++iter)
    {
        logInfo("Loading CommandSender plugin", *iter);
        IAmCommandSend* (*createFunc)();
        void* tempLibHandle = NULL;
        createFunc = getCreateFunction<IAmCommandSend*()>(*iter, tempLibHandle);

        if (!createFunc)
        {
            logInfo("Entry point of CommandPlugin not found", *iter);
            continue;
        }

        IAmCommandSend* commander = createFunc();

        if (!commander)
        {
            logInfo("CommandPlugin initialization failed. Entry Function not callable");
            dlclose(tempLibHandle);
            continue;
        }

        //check libversion
        std::string version;
        commander->getInterfaceVersion(version);
        uint16_t minorVersion, majorVersion;
        std::istringstream(version.substr(0, 1)) >> majorVersion;
        std::istringstream(version.substr(2, 1)) >> minorVersion;

        if (majorVersion < REQUIRED_INTERFACE_VERSION_MAJOR || ((majorVersion == REQUIRED_INTERFACE_VERSION_MAJOR) && (minorVersion > REQUIRED_INTERFACE_VERSION_MINOR)))
        {
            logInfo("CommandInterface initialization failed. Version of Interface to old");
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
    //unloadLibraries();
}

am_Error_e CAmCommandSender::startupInterfaces(CAmCommandReceiver *iCommandReceiver)
{
    mCommandReceiver = iCommandReceiver;
    am_Error_e returnError = E_OK;

    std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();
    std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();
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
    CALL_ALL_INTERFACES(cbMainConnectionStateChanged(connectionID,connectionState))
}

void CAmCommandSender::cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty)
{
    CALL_ALL_INTERFACES(cbMainSinkSoundPropertyChanged(sinkID,SoundProperty))
}

void CAmCommandSender::cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty)
{
    CALL_ALL_INTERFACES(cbMainSourceSoundPropertyChanged(sourceID,SoundProperty))
}

void CAmCommandSender::cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    CALL_ALL_INTERFACES(cbSinkAvailabilityChanged(sinkID,availability))
}

void CAmCommandSender::cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    CALL_ALL_INTERFACES(cbSourceAvailabilityChanged(sourceID,availability))
}

void CAmCommandSender::cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    CALL_ALL_INTERFACES(cbVolumeChanged(sinkID,volume))
}

void CAmCommandSender::cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    CALL_ALL_INTERFACES(cbSinkMuteStateChanged(sinkID,muteState))
}

void CAmCommandSender::cbSystemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
    CALL_ALL_INTERFACES(cbSystemPropertyChanged(SystemProperty))
}

void CAmCommandSender::cbTimingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
    CALL_ALL_INTERFACES(cbTimingInformationChanged(mainConnection,time))
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

    //create a list of handles
    std::vector<uint16_t> listStartupHandles;
    for (size_t i = 0; i < mListInterfaces.size(); i++)
    {
        listStartupHandles.push_back(mCommandReceiver->getStartupHandle());
    }

    //set the receiver ready to wait for replies
    mCommandReceiver->waitOnStartup(true);

    //now do the calls
    std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();
    std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();
    std::vector<uint16_t>::const_iterator handleIter(listStartupHandles.begin());
    for (; iter < iterEnd; ++iter)
    {
        (*iter)->setCommandReady(*(handleIter++));
    }
}

void CAmCommandSender::setCommandRundown()
{
    mCommandReceiver->waitOnRundown(false);
    //create a list of handles
    std::vector<uint16_t> listStartupHandles;
    for (size_t i = 0; i < mListInterfaces.size(); i++)
    {
        listStartupHandles.push_back(mCommandReceiver->getRundownHandle());
    }

    //set the receiver ready to wait for replies
    mCommandReceiver->waitOnRundown(true);

    //now do the calls
    std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();
    std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();
    std::vector<uint16_t>::const_iterator handleIter(listStartupHandles.begin());
    for (; iter < iterEnd; ++iter)
    {
        (*iter)->setCommandRundown(*(handleIter++));
    }
}

void CAmCommandSender::getInterfaceVersion(std::string & version) const
{
    version = CommandSendVersion;
}

am_Error_e am::CAmCommandSender::getListPlugins(std::vector<std::string> & interfaces) const
{
    interfaces = mListLibraryNames;
    return (E_OK);
}

void CAmCommandSender::unloadLibraries(void)
{
    std::vector<void*>::iterator iterator = mListLibraryHandles.begin();
    for (; iterator < mListLibraryHandles.end(); ++iterator)
    {
        dlclose(*iterator);
    }
    mListLibraryHandles.clear();
}
}
