/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file HookEngine.h
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

#ifndef HOOKENGINE_H_
#define HOOKENGINE_H_

#include "audioManagerIncludes.h"

class AudioManagerCore;
class Queue;

/**These define the different types of Hooks available in the system
 *
 */
typedef enum genHook {
	HOOK_DOMAIN_REGISTER, //!< HOOK_DOMAIN_REGISTER
	HOOK_DOMAIN_DEREGISTER, //!< HOOK_DOMAIN_DEREGISTER
	HOOK_SOURCE_REGISTER, //!< HOOK_SOURCE_REGISTER
	HOOK_SOURCE_DEREGISTER, //!< HOOK_SOURCE_DEREGISTER
	HOOK_SINK_REGISTER, //!< HOOK_SINK_REGISTER
	HOOK_SINK_DEREGISTER, //!< HOOK_SINK_DEREGISTER
	HOOK_GATEWAY_REGISTER, //!< HOOK_GATEWAY_REGISTER
	HOOK_GATEWAY_DERGISTER, //!< HOOK_GATEWAY_DERGISTER
	HOOK_SYSTEM_READY, //!< HOOK_SYSTEM_READY
	HOOK_SYSTEM_DOWN, //!< HOOK_SYSTEM_DOWN
	HOOK_USER_CONNECTION_REQUEST, //!< HOOK_USER_CONNECTION_REQUEST
	HOOK_USER_DISCONNECTION_REQUEST, //!< HOOK_USER_DISCONNECTION_REQUEST
	HOOK_CONNECTION_REQUEST, //!< HOOK_CONNECTION_REQUEST
	HOOK_DISCONNECTION_REQUEST, //!< HOOK_DISCONNECTION_REQUEST
	HOOK_ROUTING_REQUEST, //!< HOOK_ROUTING_REQUEST
	HOOK_ROUTING_COMPLETE, //!< HOOK_ROUTING_COMPLETE
	HOOK_VOLUME_CHANGE, //!< HOOK_VOLUME_CHANGE
	HOOK_MUTE_SOURCE, //!< HOOK_MUTE_SOURCEDataBaseHandler
	HOOK_UNMUTE_SOURCE, //!< HOOK_UNMUTE_SOURCE
	HOOK_MUTE_SINK, //!< HOOK_MUTE_SINK
	HOOK_UNMUTE_SINK, //!< HOOK_UNMUTE_SINK
	HOOK_INTERRUPT_REQUEST
//!< HOOK_INTERRUPT_REQUEST
} genHook_t;

class HookHandler;

/**This is the base class for all HookPlugins
 * To implement a HookPlugin, a QTPlugin HookPluginFactory Class needs to be created which factors an instance of a class that is derived from
 * BaseHook.
 * All Hooks that are possible are implemented here as virtual functions. To use a hook, the derived class needs simply to overwrite the
 * virtual hook functions. This means a plugin can only have one hook function for each hook (more than one does not make sense anyway).
 * Each plugin need to implement a Init function. Within this function the hooks that the plugin wants to use need to be registered.
 *
 */
class BaseHook {
public:
	BaseHook();
	virtual ~BaseHook();
	/**This function is used to register the HookHandler in the plugin.
	 *
	 * @param engine pointer to the instance of the HookHandler
	 * @return GEN_OK on success
	 */
	genError_t registerHookEngine(HookHandler* engine);

	/**This is the init function. Register your hooks here, like for example register to the domainregisterhook with prio 10:
	 * @code m_hookhandler->registerHook(10,HOOK_DOMAIN_REGISTER,this); @endcode
	 *
	 * @return GEN_OK on success
	 */
	virtual genError_t InitHook(void)=0;

	/**Deinit function. If you need to store or cleaup -> here is the right place to do it
	 *
	 * @return GEN_OK on success
	 */
	virtual genError_t DeinitHook(void)=0;

	/**Retrieve the name of the plugin
	 *  max length of the name: 40 chars
	 *  @param PluginName buffer to write the name to
	 *  @return GEN_OK on success
	 */
	virtual genError_t returnPluginName(char* PluginName)=0;

	void registerAudioManagerCore(AudioManagerCore* core);

	virtual genHookResult_t hookDomainRegister(char* Name, domain_t ID) {
		(void) Name;
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookDomainDeregister(domain_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookSinkRegister(char* Name, sink_t ID) {
		(void) Name;
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookSinkDeregister(sink_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookSourceRegister(char* Name, source_t ID) {
		(void) Name;
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookSourceDeregister(source_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookGatewayRegister(char* Name, gateway_t ID) {
		(void) Name;
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookGatewayDeregister(gateway_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookSystemReady(void) {
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookSystemDown(void) {
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookUserConnectionRequest(Queue* queue,
			source_t SourceID, sink_t SinkID) {
		(void) queue;
		(void) SourceID;
		(void) SinkID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookUserDisconnectionRequest(Queue* queue,
			connection_t connID) {
		(void) queue;
		(void) connID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookConnectionRequest(source_t SourceID,
			sink_t SinkID) {
		(void) SourceID;
		(void) SinkID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookDisconnectionRequest(connection_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookRoutingRequest(bool onlyfree, source_t source,
			sink_t sink, std::list<genRoute_t>* ReturnList) {
		(void) onlyfree;
		(void) source;
		(void) sink;
		(void) ReturnList;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookRoutingComplete(genRoute_t route) {
		(void) route;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookVolumeChange(volume_t newVolume, sink_t SinkID) {
		(void) newVolume;
		(void) SinkID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookMuteSource(source_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookUnmuteSource(source_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookMuteSink(sink_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookUnmuteSink(sink_t ID) {
		(void) ID;
		return HOOK_UNUSED;
	}
	;
	virtual genHookResult_t hookInterruptRequest(Queue* queue,
			source_t interruptSource, sink_t sink, genInt_t* interruptID) {
		(void) queue;
		(void) interruptSource;
		(void) sink;
		(void) interruptID;
		return HOOK_UNUSED;
	}
	;

protected:
	HookHandler* m_hookhandler;
	AudioManagerCore* m_core;
};

/**This class handles all the hooks.
 * It maintains a list of all registered hooks and calls them if desired.
 *
 */
class HookHandler {
public:
	HookHandler();
	virtual ~HookHandler();
	/**By calling this functions, all Plugins that are related to hooks are automatically loaded
	 * The init function is called as well, so that the plugins can register their hooks
	 */
	void loadHookPlugins();

	/**This function is used to register a hook
	 *
	 * @param prio	the prio (between 0 and 100, 100 is max)
	 * @param hookType gives the type of the hook. Must match the hook callback of course !
	 * @param hookClass This is a pointer to the Class registering the hook. Usually this.
	 * @return GEN_OK on success
	 */
	genError_t registerHook(hookprio_t prio, genHook_t hookType,
			BaseHook* hookClass);

	void registerAudioManagerCore(AudioManagerCore* core);

	/**All functions starting with fire are called to execute the hook. They will go throDataBaseHandler::ugh the list
	 * of registered hooks and call them after the priorities.
	 */

	genError_t fireHookDomainRegister(char* Name, domain_t ID);
	genError_t fireHookDomainDeregister(domain_t ID);
	genError_t fireHookSinkRegister(char* Name, sink_t ID);
	genError_t fireHookSinkDeregister(sink_t ID);
	genError_t fireHookSourceRegister(char* Name, source_t ID);
	genError_t fireHookSourceDeregister(source_t ID);
	genError_t fireHookGatewayRegister(char* Name, gateway_t ID);
	genError_t fireHookGatewayDeregister(gateway_t ID);
	genError_t fireHookSystemReady(void);
	genError_t fireHookSystemDown(void);
	genError_t fireHookConnectionRequest(source_t SourceID, sink_t SinkID);
	genError_t fireHookDisconnectionRequest(connection_t ID);
	genError_t fireHookUserConnectionRequest(Queue* queue, source_t SourceID,
			sink_t SinkID);
	genError_t fireHookUserDisconnectionRequest(Queue* queue,
			connection_t connID);
	genError_t fireHookRoutingRequest(bool onlyfree, source_t SourceID, sink_t SinkID, std::list<genRoute_t>* ReturnList);
	genError_t fireHookRoutingComplete(genRoute_t route);
	genError_t fireHookVolumeChange(volume_t newVolume, sink_t SinkID);
	genError_t fireHookMuteSource(source_t ID);
	genError_t fireHookUnmuteSource(source_t ID);
	genError_t fireHookMuteSink(sink_t ID);
	genError_t fireHookUnmuteSink(sink_t ID);
	genError_t fireHookInterruptRequest(Queue* queue, source_t interruptSource,
			sink_t sink, genInt_t* interruptID);

private:
	/**Struct for managing the hookLists
	 * This struct holds the pointer to the instance of the hook to be called and the priority after that the list ist sorted.
	 *
	 */
	struct prioList {
		BaseHook* hook;
		hookprio_t prio;
	};

	std::list<prioList> m_domainRegisterList;
	std::list<prioList> m_domainDeregisterList;
	std::list<prioList> m_sinkRegisterList;
	std::list<prioList> m_sinkDeregisterList;
	std::list<prioList> m_sourceRegisterList;
	std::list<prioList> m_sourceDeregisterList;
	std::list<prioList> m_gatewayRegisterList;
	std::list<prioList> m_gatewayDeregisterList;
	std::list<prioList> m_systemReadyList;
	std::list<prioList> m_systemDownList;
	std::list<prioList> m_connectionRequestList;
	std::list<prioList> m_disconnectionReuestList;
	std::list<prioList> m_userConnectionRequestList;
	std::list<prioList> m_userDisconnectionReuestList;
	std::list<prioList> m_routingRequestList;
	std::list<prioList> m_routingCompleteList;
	std::list<prioList> m_volumeChangeList;
	std::list<prioList> m_muteSourceList;
	std::list<prioList> m_unmuteSourceList;
	std::list<prioList> m_muteSinkList;
	std::list<prioList> m_unmuteSinkList;
	std::list<prioList> m_interruptRequestList;

	AudioManagerCore* m_core;
	std::list<BaseHook*> m_listofPlugins;
};

#endif /* HOOKENGINE_H_ */
