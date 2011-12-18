/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file CommandSender.cpp
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


#include "CommandSender.h"
#include <dirent.h>
#include <dlt/dlt.h>
#include "PluginTemplate.h"
using namespace am;

DLT_IMPORT_CONTEXT(DLT_CONTEXT)

//!< macro to call all interfaces
#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<CommandSendInterface*>::iterator iter = mListInterfaces.begin();	 \
		std::vector<CommandSendInterface*>::iterator iterEnd = mListInterfaces.end();	 \
		for (; iter<iterEnd;++iter)													 	 \
		{																				 \
			(*iter)->__VA_ARGS__;													 	 \
		}

CommandSender::CommandSender(const std::vector<std::string>& listOfPluginDirectories)
	:mListInterfaces(),
	 mListLibraryHandles()
{
	std::vector<std::string> sharedLibraryNameList;
    std::vector<std::string>::const_iterator dirIter = listOfPluginDirectories.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listOfPluginDirectories.end();

    // search communicator plugins in configured directories
    for (; dirIter < dirIterEnd; ++dirIter)
    {
		const char* directoryName = dirIter->c_str();
		DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("Searching for CommandPlugins in"),DLT_STRING(directoryName));
		DIR *directory = opendir(directoryName);

		if (!directory)
		{
			DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("Error opening directory "),DLT_STRING(directoryName));
		}

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
			unsigned char entryType = itemInDirectory->d_type;
			std::string entryName = itemInDirectory->d_name;

			bool regularFile = (entryType == DT_REG);
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
    	DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("Loading CommandSender plugin"),DLT_STRING(iter->c_str()));
        CommandSendInterface* (*createFunc)();
        void* tempLibHandle=NULL;
        createFunc = getCreateFunction<CommandSendInterface*()>(*iter,tempLibHandle);

        if (!createFunc)
        {
            DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("Entry point of CommandPlugin not found"),DLT_STRING(iter->c_str()));
            continue;
        }

        CommandSendInterface* commander = createFunc();

        if (!commander)
        {
        	DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("CommandPlugin initialization failed. Entry Function not callable"));
            continue;
        }

        mListInterfaces.push_back(commander);
        mListLibraryHandles.push_back(tempLibHandle);
    }
}

CommandSender::~CommandSender()
{
	unloadLibraries();
}


am_Error_e CommandSender::stopInterface()
{
	am_Error_e returnError=E_OK;

	std::vector<CommandSendInterface*>::iterator iter = mListInterfaces.begin();
	std::vector<CommandSendInterface*>::iterator iterEnd = mListInterfaces.end();
	for (; iter<iterEnd;++iter)
	{
		am_Error_e error=(*iter)->stopInterface();
		if (error!= E_OK)
		{
			returnError=error;
		}
	}
	return returnError;
}


am_Error_e CommandSender::startupInterface(CommandReceiveInterface *commandreceiveinterface)
{
	am_Error_e returnError=E_OK;

	std::vector<CommandSendInterface*>::iterator iter = mListInterfaces.begin();
	std::vector<CommandSendInterface*>::iterator iterEnd = mListInterfaces.end();
	for (; iter<iterEnd;++iter)
	{
		am_Error_e error=(*iter)->startupInterface(commandreceiveinterface);
		if (error!= E_OK)
		{
			returnError=error;
		}
	}
	return returnError;
}


void CommandSender::cbCommunicationReady()
{
	CALL_ALL_INTERFACES(cbCommunicationReady())
}

void CommandSender::cbCommunicationRundown()
{
	CALL_ALL_INTERFACES(cbCommunicationRundown())
}

void CommandSender::cbNumberOfMainConnectionsChanged()
{
	CALL_ALL_INTERFACES(cbNumberOfMainConnectionsChanged())
}

void CommandSender::cbNumberOfSinksChanged()
{
	CALL_ALL_INTERFACES(cbNumberOfSinksChanged())
}

void CommandSender::cbNumberOfSourcesChanged()
{
	CALL_ALL_INTERFACES(cbNumberOfSourcesChanged())
}

void CommandSender::cbNumberOfSinkClassesChanged()
{
	CALL_ALL_INTERFACES(cbNumberOfSinkClassesChanged())
}

void CommandSender::cbNumberOfSourceClassesChanged()
{
	CALL_ALL_INTERFACES(cbNumberOfSourceClassesChanged())
}


void CommandSender::cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
	CALL_ALL_INTERFACES(cbMainConnectionStateChanged(connectionID,connectionState))
}



void CommandSender::cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty)
{
	CALL_ALL_INTERFACES(cbMainSinkSoundPropertyChanged(sinkID,SoundProperty))
}



void CommandSender::cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
	CALL_ALL_INTERFACES(cbMainSourceSoundPropertyChanged(sourceID,SoundProperty))
}



void CommandSender::cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
	CALL_ALL_INTERFACES(cbSinkAvailabilityChanged(sinkID,availability))
}



void CommandSender::cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
	CALL_ALL_INTERFACES(cbSourceAvailabilityChanged(sourceID,availability))
}



void CommandSender::cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
	CALL_ALL_INTERFACES(cbVolumeChanged(sinkID,volume))
}



void CommandSender::cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
	CALL_ALL_INTERFACES(cbSinkMuteStateChanged(sinkID,muteState))
}



void CommandSender::cbSystemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
	CALL_ALL_INTERFACES(cbSystemPropertyChanged(SystemProperty))
}



void CommandSender::cbTimingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
	CALL_ALL_INTERFACES(cbTimingInformationChanged(mainConnection,time))
}

void CommandSender::unloadLibraries(void)
{
	std::vector<void*>::iterator iterator=mListLibraryHandles.begin();
	for(;iterator<mListLibraryHandles.end();++iterator)
	{
		dlclose(*iterator);
	}
	mListLibraryHandles.clear();
}





