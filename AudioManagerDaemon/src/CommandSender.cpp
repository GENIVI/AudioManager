/*
 * CommandSender.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: christian
 */

#include "CommandSender.h"
#include "command/CommandSendInterface.h"
#include "pluginTemplate.h"
using namespace am;

#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<CommandSendInterface*>::iterator iter = mListInterfaces.begin();	 \
		std::vector<CommandSendInterface*>::iterator iterEnd = mListInterfaces.end();	 \
		for (; iter<iterEnd;++iter)													 \
		{																				 \
			(*iter)->__VA_ARGS__;													 	 \
		}


const char* commandPluginDirectories[] = { "/home/christian/workspace/gitserver/build/plugins/command"};
uint16_t commandPluginDirectoriesCount = sizeof(commandPluginDirectories) / sizeof(commandPluginDirectories[0]);

CommandSender::CommandSender()
{
	std::vector<std::string> sharedLibraryNameList;

    // search communicator plugins in configured directories
    for (uint16_t dirIndex = 0; dirIndex < commandPluginDirectoriesCount; dirIndex++)
    {
		const char* directoryName = commandPluginDirectories[dirIndex];
		//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Searching for HookPlugins in"),DLT_STRING(directoryName));
		DIR *directory = opendir(directoryName);

		if (!directory)
		{
			//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Error opening directory "),DLT_STRING(dirName.c_str()));
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
			//	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("PluginSearch adding file "),DLT_STRING(entryName.c_str()));
			  std::string name(directoryName);
			  sharedLibraryNameList.push_back(name + "/" + entryName);
			}
			else
			{
			//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("PluginSearch ignoring file "),DLT_STRING(entryName.c_str()));
			}
        }

          closedir(directory);
    }

    // iterate all communicator plugins and start them
    std::vector<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::vector<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter < iterEnd; ++iter)
    {
    	//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Loading Hook plugin"),DLT_STRING(iter->c_str()));
        CommandSendInterface* (*createFunc)();
        createFunc = getCreateFunction<CommandSendInterface*()>(*iter);

        if (!createFunc)
        {
           // DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Entry point of Communicator not found"));
            continue;
        }

        CommandSendInterface* commander = createFunc();

        if (!commander)
        {
        	//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("HookPlugin initialization failed. Entry Function not callable"));
            continue;
        }

        mListInterfaces.push_back(commander);
    }
}

CommandSender::~CommandSender()
{
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



