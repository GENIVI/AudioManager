/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file CAmCommandSender.cpp
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

#include "CAmCommandSender.h"
#include <dirent.h>
#include <sstream>
#include <string>
#include "CAmCommandReceiver.h"
#include "TAmPluginTemplate.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

#define REQUIRED_INTERFACE_VERSION_MAJOR 1
#define REQUIRED_INTERFACE_VERSION_MINOR 0

//!< macro to call all interfaces
#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();	 \
		std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();	 \
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
            continue;
        }

        mListInterfaces.push_back(commander);
        mListLibraryHandles.push_back(tempLibHandle);
        mListLibraryNames.push_back(iter->c_str());
    }
}

CAmCommandSender::~CAmCommandSender()
{
    unloadLibraries();
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
    return returnError;
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
    std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();
    std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        (*iter)->setCommandReady(mCommandReceiver->getStartupHandle());
    }
    mCommandReceiver->waitOnStartup(true);
}

void CAmCommandSender::setCommandRundown()
{
    mCommandReceiver->waitOnRundown(false);
    std::vector<IAmCommandSend*>::iterator iter = mListInterfaces.begin();
    std::vector<IAmCommandSend*>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        (*iter)->setCommandRundown(mCommandReceiver->getRundownHandle());
    }
    mCommandReceiver->waitOnRundown(true);
}

void CAmCommandSender::getInterfaceVersion(std::string & version) const
{
    version = CommandSendVersion;
}

am_Error_e am::CAmCommandSender::getListPlugins(std::vector<std::string> & interfaces) const
{
    interfaces = mListLibraryNames;
    return E_OK;
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
