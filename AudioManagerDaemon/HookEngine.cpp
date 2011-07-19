/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file HookEngine.cpp
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 *
 */

#include "HookEngine.h"

/**
 * this path needs to be adjusted to whatever is suitable on the system
 */
const char* hookPluginDirectories[] = { "/home/blacky/new_workspace/AudioManager/build/plugins"};
uint hookPluginDirectoriesCount = sizeof(hookPluginDirectories) / sizeof(hookPluginDirectories[0]);

#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>

template<class T>T* getCreateFunction(std::string libname) {

	// cut off directories
	char* fileWithPath = const_cast<char*>(libname.c_str());
	std::string libFileName = basename(fileWithPath);

	// cut off "lib" in front and cut off .so end"
	std::string createFunctionName = libFileName.substr(3, libFileName.length() - 6) + "Factory";
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Lib entry point name "),DLT_STRING(createFunctionName.c_str()));

	// open library#include <unistd.h>
	void *libraryHandle;
	dlerror(); // Clear any existing error
	libraryHandle = dlopen(libname.c_str(), RTLD_NOW /*LAZY*/);
	const char* dlopen_error = dlerror();
	if (!libraryHandle || dlopen_error)
	{
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("dlopen failed"),DLT_STRING(dlopen_error));
		return 0;
	}

	// get entry point from shared lib
	dlerror(); // Clear any existing error
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("loading external function with name"),DLT_STRING(createFunctionName.c_str()));

	union
	{
		void* voidPointer;
		T* typedPointer;
	} functionPointer;

	// Note: direct cast is not allowed by ISO C++. e.g.
	// T* createFunction = reinterpret_cast<T*>(dlsym(libraryHandle, createFunctionName.c_str()));
	// compiler warning: "forbids casting between pointer-to-function and pointer-to-object"

	functionPointer.voidPointer = dlsym(libraryHandle, createFunctionName.c_str());
	T* createFunction = functionPointer.typedPointer;

	const char* dlsym_error = dlerror();
	if (!createFunction || dlsym_error)
	{
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Failed to load shared lib entry point"),DLT_STRING(dlsym_error));
	}

	return createFunction;
}


BaseHook::BaseHook() {
}

BaseHook::~BaseHook() {
}

HookHandler::HookHandler() {
}

HookHandler::~HookHandler() {
	for (std::list<BaseHook*>::iterator hook = m_listofPlugins.begin(); hook != m_listofPlugins.end(); hook++) {
		(*hook)->DeinitHook();
	}
}

void BaseHook::registerAudioManagerCore(AudioManagerCore* core) {
	m_core = core;
}

genError_t BaseHook::registerHookEngine(HookHandler* engine) {
	m_hookhandler = engine;
	return GEN_OK;
}

genError_t HookHandler::registerHook(hookprio_t prio, genHook_t hookType, BaseHook* hookClass) {
	prioList newEntry;
	std::list<prioList>* list;

	if (prio < 0 || prio > 100) {
		DLT_LOG( AudioManager, DLT_LOG_WARN, DLT_STRING("Register Hook: Priority out of range: "), DLT_INT(prio));
		return GEN_OUTOFRANGE;
	}

	newEntry.prio = prio;
	newEntry.hook = hookClass;

	switch (hookType) {
	case HOOK_DOMAIN_REGISTER:
		list = &m_domainRegisterList;
		break;
	case HOOK_DOMAIN_DEREGISTER:
		list = &m_domainDeregisterList;
		break;
	case HOOK_SINK_REGISTER:
		list = &m_sinkRegisterList;
		break;
	case HOOK_SINK_DEREGISTER:
		list = &m_sinkDeregisterList;
		break;
	case HOOK_SOURCE_REGISTER:
		list = &m_sourceRegisterList;
		break;
	case HOOK_SOURCE_DEREGISTER:
		list = &m_sourceDeregisterList;
		break;
	case HOOK_GATEWAY_REGISTER:
		list = &m_gatewayRegisterList;
		break;
	case HOOK_GATEWAY_DERGISTER:
		list = &m_gatewayDeregisterList;
		break;
	case HOOK_ROUTING_REQUEST:
		list = &m_routingRequestList;
		break;
	case HOOK_ROUTING_COMPLETE:
		list = &m_routingCompleteList;
		break;
	case HOOK_SYSTEM_READY:
		list = &m_systemReadyList;
		break;
	case HOOK_SYSTEM_DOWN:
		list = &m_systemDownList;
		break;
	case HOOK_VOLUME_CHANGE:
		list = &m_volumeChangeList;
		break;
	case HOOK_MUTE_SOURCE:
		list = &m_muteSourceList;
		break;
	case HOOK_UNMUTE_SOURCE:
		list = &m_unmuteSourceList;
		break;
	case HOOK_MUTE_SINK:
		list = &m_muteSinkList;
		break;
	case HOOK_UNMUTE_SINK:
		list = &m_unmuteSinkList;
		break;
	case HOOK_USER_CONNECTION_REQUEST:
		list = &m_userConnectionRequestList;
		break;
	case HOOK_USER_DISCONNECTION_REQUEST:
		list = &m_userDisconnectionReuestList;
		break;
	case HOOK_CONNECTION_REQUEST:
		list = &m_connectionRequestList;
		break;
	case HOOK_DISCONNECTION_REQUEST:
		list = &m_disconnectionReuestList;
		break;
	case HOOK_INTERRUPT_REQUEST:
		list = &m_interruptRequestList;
		break;
	default:
		DLT_LOG(AudioManager, DLT_LOG_WARN, DLT_STRING("Trying to register unknown Hook "));
		return GEN_OUTOFRANGE;
	}

	int index = 0;
	std::list<prioList>::iterator l;
	for (l = list->begin(); l != list->end(); l++) {
		if (l->prio > prio) {
			index++;
		}
	}
	std::advance(l, index);
	list->insert(l, newEntry);
	//TODO test the sorting of the hooks with more than one plugin

	return GEN_OK;
}

genError_t HookHandler::fireHookDomainRegister(char* Name, domain_t ID) {
	for (std::list<prioList>::iterator hook = m_domainRegisterList.begin(); hook != m_domainDeregisterList.end(); hook++) {
		if (hook->hook->hookDomainRegister(Name, ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookDomainDeregister(domain_t ID) {
	for (std::list<prioList>::iterator hook = m_domainDeregisterList.begin(); hook != m_domainDeregisterList.end(); hook++) {
		if (hook->hook->hookDomainDeregister(ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookSinkRegister(char* Name, sink_t ID) {
	for (std::list<prioList>::iterator hook = m_sinkRegisterList.begin(); hook != m_sinkRegisterList.end(); hook++) {
		if (hook->hook->hookSinkRegister(Name, ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookSinkDeregister(sink_t ID) {
	for (std::list<prioList>::iterator hook = m_sinkDeregisterList.begin(); hook != m_sinkDeregisterList.end(); hook++) {
		if (hook->hook->hookSinkDeregister(ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookSourceRegister(char* Name, source_t ID) {
	for (std::list<prioList>::iterator hook = m_sourceRegisterList.begin(); hook != m_sourceRegisterList.end(); hook++) {
		if (hook->hook->hookSourceRegister(Name, ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookSourceDeregister(source_t ID) {
	for (std::list<prioList>::iterator hook = m_sourceDeregisterList.begin(); hook != m_sourceDeregisterList.end(); hook++) {
		if (hook->hook->hookSourceDeregister(ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookGatewayRegister(char* Name, gateway_t ID) {
	for (std::list<prioList>::iterator hook = m_gatewayRegisterList.begin(); hook != m_gatewayRegisterList.end(); hook++) {
		if (hook->hook->hookGatewayRegister(Name, ID) == HOOK_STOP)
			break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookGatewayDeregister(gateway_t ID) {
	for (std::list<prioList>::iterator hook = m_gatewayDeregisterList.begin(); hook != m_gatewayDeregisterList.end(); hook++) {
		if (hook->hook->hookGatewayDeregister(ID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookSystemReady(void) {
	for (std::list<prioList>::iterator hook = m_systemReadyList.begin(); hook != m_systemReadyList.end(); hook++) {
		if (hook->hook->hookSystemReady() == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookSystemDown(void) {
	for (std::list<prioList>::iterator hook = m_systemDownList.begin(); hook != m_systemDownList.end(); hook++) {
		if (hook->hook->hookSystemDown() == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookConnectionRequest(source_t SourceID, sink_t SinkID) {
	for (std::list<prioList>::iterator hook = m_connectionRequestList.begin(); hook != m_connectionRequestList.end(); hook++) {
		if (hook->hook->hookConnectionRequest(SourceID, SinkID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookDisconnectionRequest(connection_t ID) {
	for (std::list<prioList>::iterator hook = m_disconnectionReuestList.begin(); hook != m_disconnectionReuestList.end(); hook++) {
		if (hook->hook->hookDisconnectionRequest(ID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookUserConnectionRequest(Queue* queue, source_t SourceID, sink_t SinkID) {
	for (std::list<prioList>::iterator hook = m_userConnectionRequestList.begin(); hook != m_userConnectionRequestList.end(); hook++) {
		if (hook->hook->hookUserConnectionRequest(queue, SourceID, SinkID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookUserDisconnectionRequest(Queue* queue, connection_t connID) {
	for (std::list<prioList>::iterator hook = m_userDisconnectionReuestList.begin(); hook != m_userDisconnectionReuestList.end(); hook++) {
		if (hook->hook->hookUserDisconnectionRequest(queue, connID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookRoutingRequest(bool onlyfree, source_t source, sink_t sink, std::list<genRoute_t>* ReturnList) {
	for (std::list<prioList>::iterator hook = m_routingRequestList.begin(); hook != m_routingRequestList.end(); hook++) {
		if (hook->hook->hookRoutingRequest(onlyfree, source, sink, ReturnList) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

//todo change type
genError_t HookHandler::fireHookRoutingComplete(genRoute_t route) {
	for (std::list<prioList>::iterator hook = m_routingCompleteList.begin(); hook != m_routingCompleteList.end(); hook++) {
		if (hook->hook->hookRoutingComplete(route) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookVolumeChange(volume_t newVolume, sink_t SinkID) {
	for (std::list<prioList>::iterator hook = m_volumeChangeList.begin(); hook != m_volumeChangeList.end(); hook++) {
		if (hook->hook->hookVolumeChange(newVolume, SinkID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookMuteSource(source_t ID) {
	for (std::list<prioList>::iterator hook = m_muteSourceList.begin(); hook != m_muteSourceList.end(); hook++) {
		if (hook->hook->hookMuteSource(ID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookUnmuteSource(source_t ID) {
	for (std::list<prioList>::iterator hook = m_unmuteSourceList.begin(); hook != m_unmuteSourceList.end(); hook++) {
		if (hook->hook->hookUnmuteSource(ID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookMuteSink(sink_t ID) {
	for (std::list<prioList>::iterator hook = m_muteSinkList.begin(); hook != m_muteSinkList.end(); hook++) {
		if (hook->hook->hookMuteSink(ID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookUnmuteSink(sink_t ID) {
	for (std::list<prioList>::iterator hook = m_unmuteSinkList.begin(); hook != m_unmuteSinkList.end(); hook++) {
		if (hook->hook->hookUnmuteSink(ID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

genError_t HookHandler::fireHookInterruptRequest(Queue* queue, source_t interruptSource, sink_t sink, genInt_t* interruptID) {
	for (std::list<prioList>::iterator hook = m_interruptRequestList.begin(); hook != m_interruptRequestList.end(); hook++) {
		if (hook->hook->hookInterruptRequest(queue, interruptSource, sink, interruptID) == HOOK_STOP)
		break;
	}
	return GEN_OK;
}

void HookHandler::registerAudioManagerCore(AudioManagerCore* core) {
	m_core = core;
}

void HookHandler::loadHookPlugins() {

	std::list<std::string> sharedLibraryNameList;

    // search communicator plugins in configured directories
    for (uint dirIndex = 0; dirIndex < hookPluginDirectoriesCount; ++dirIndex) {
        const char* directoryName = hookPluginDirectories[dirIndex];
        DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Searching for HookPlugins in"),DLT_STRING(directoryName));
        std::list<std::string> newList=m_core->getSharedLibrariesFromDirectory(directoryName);
        sharedLibraryNameList.insert(sharedLibraryNameList.end(),newList.begin(),newList.end());
    }


    // iterate all communicator plugins and start them
    std::list<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::list<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter != iterEnd; ++iter)
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Loading Hook plugin"),DLT_STRING(iter->c_str()));

        BaseHook* (*createFunc)();
        createFunc = getCreateFunction<BaseHook*()>(*iter);

        if (!createFunc) {
            DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Entry point of Communicator not found"));
            continue;
        }

        BaseHook* HookPlugin = createFunc();

        if (!HookPlugin) {
        	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("HookPlugin initialization failed. Entry Function not callable"));
            continue;
        }

		HookPlugin->registerHookEngine(this);
		HookPlugin->registerAudioManagerCore(m_core);
		HookPlugin->InitHook();
		char pName[40];
		HookPlugin->returnPluginName(pName);
		DLT_LOG( AudioManager, DLT_LOG_INFO, DLT_STRING("Registered Hook Plugin:"), DLT_STRING(pName));
        m_listofPlugins.push_back(HookPlugin);
    }

}

