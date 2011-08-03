/*
 * CommandHandler.cpp
 *
 *  Created on: Jul 27, 2011
 *      Author: christian
 */

#include "CommandHandler.h"
#include "audioManagerIncludes.h"

const char* commandPluginDirectories[] = { "/home/christian/workspace/gitserver/build/plugins/command"};
uint commandPluginDirectoriesCount = sizeof(commandPluginDirectories) / sizeof(commandPluginDirectories[0]);

CommandHandler::CommandHandler(AudioManagerCore* core) :m_core(core) {
	// TODO Auto-generated constructor stub

}

CommandHandler::~CommandHandler() {
	// TODO Auto-generated destructor stub
}

void CommandHandler::registerReceiver(CommandReceiveInterface *iface) {
	m_receiver=iface;
}

void CommandHandler::loadPlugins(){
	std::list<std::string> sharedLibraryNameList;

	for (uint dirIndex = 0; dirIndex < commandPluginDirectoriesCount; ++dirIndex) {
        const char* directoryName = commandPluginDirectories[dirIndex];
        DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Searching for Command in"),DLT_STRING(directoryName));
        std::list<std::string> newList=m_core->getSharedLibrariesFromDirectory(directoryName);
        sharedLibraryNameList.insert(sharedLibraryNameList.end(),newList.begin(),newList.end());
    }


    // iterate all communicator plugins and start them
    std::list<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::list<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter != iterEnd; ++iter)
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Loading Command plugin"),DLT_STRING(iter->c_str()));

    	CommandSendInterface* (*createFunc)();
        createFunc = getCreateFunction<CommandSendInterface*()>(*iter);

        if (!createFunc) {
            DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Entry point of Communicator not found"));
            continue;
        }

        CommandSendInterface* CommandPlugin = createFunc();


        if (!CommandPlugin) {
        	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("RoutingPlugin initialization failed. Entry Function not callable"));
            continue;
        }


        CommandPlugin->startupInterface(m_receiver);
		DLT_LOG( AudioManager, DLT_LOG_INFO, DLT_STRING("Registered Routing Plugin"));
        m_interfaceList.push_back(CommandPlugin);
    }
}



