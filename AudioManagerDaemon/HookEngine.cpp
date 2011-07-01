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

BaseHook::BaseHook() {
}

BaseHook::~BaseHook() {
}

HookHandler::HookHandler() {
}

HookHandler::~HookHandler() {
}

void BaseHook::registerAudioManagerCore(AudioManagerCore* core) {
	m_core = core;
}

genError_t BaseHook::registerHookEngine(HookHandler* engine) {
	m_hookhandler = engine;
	return GEN_OK;
}

genError_t HookHandler::registerHook(hookprio_t prio, genHook_t hookType,
		BaseHook* hookClass) {
	prioList newEntry;
	QList<prioList>* list;

	if (prio < 0 || prio > 100) {
		DLT_LOG(AudioManager,DLT_LOG_WARN, DLT_STRING("Register Hook: Priority out of range: "), DLT_INT(prio));
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
		DLT_LOG(AudioManager,DLT_LOG_WARN, DLT_STRING("Trying to register unknown Hook "))
		;
		return GEN_OUTOFRANGE;
	}

	int index = 0;
	foreach(prioList l,*list)
		{
			if (l.prio > prio) {
				index++;
			}
		}
	list->insert(index, newEntry);
	//TODO test the sorting of the hooks with more than one plugin

	return GEN_OK;
}

genError_t HookHandler::fireHookDomainRegister(char* Name, domain_t ID) {
	foreach (prioList hook,m_domainRegisterList)
		{
			if (hook.hook->hookDomainRegister(Name, ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookDomainDeregister(domain_t ID) {
	foreach (prioList hook,m_domainDeregisterList)
		{
			if (hook.hook->hookDomainDeregister(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookSinkRegister(char* Name, sink_t ID) {
	foreach (prioList hook,m_sinkRegisterList)
		{
			if (hook.hook->hookSinkRegister(Name, ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookSinkDeregister(sink_t ID) {
	foreach (prioList hook,m_sinkDeregisterList)
		{
			if (hook.hook->hookSinkDeregister(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookSourceRegister(char* Name, source_t ID) {
	foreach (prioList hook,m_sourceRegisterList)
		{
			if (hook.hook->hookSinkRegister(Name, ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookSourceDeregister(source_t ID) {
	foreach (prioList hook,m_sourceDeregisterList)
		{
			if (hook.hook->hookSourceDeregister(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookGatewayRegister(char* Name, gateway_t ID) {
	foreach (prioList hook,m_gatewayRegisterList)
		{
			if (hook.hook->hookGatewayRegister(Name, ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookGatewayDeregister(gateway_t ID) {
	foreach (prioList hook,m_gatewayDeregisterList)
		{
			if (hook.hook->hookGatewayDeregister(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookSystemReady(void) {
	foreach (prioList hook,m_systemReadyList)
		{
			if (hook.hook->hookSystemReady() == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookSystemDown(void) {
	foreach (prioList hook,m_systemDownList)
		{
			if (hook.hook->hookSystemDown() == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookConnectionRequest(source_t SourceID,
		sink_t SinkID) {
	foreach (prioList hook,m_connectionRequestList)
		{
			if (hook.hook->hookConnectionRequest(SourceID, SinkID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookDisconnectionRequest(connection_t ID) {
	foreach (prioList hook,m_disconnectionReuestList)
		{
			if (hook.hook->hookDisconnectionRequest(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookUserConnectionRequest(Queue* queue,
		source_t SourceID, sink_t SinkID) {
	foreach (prioList hook,m_userConnectionRequestList)
		{
			if (hook.hook->hookUserConnectionRequest(queue, SourceID, SinkID)
					== HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookUserDisconnectionRequest(Queue* queue,
		connection_t connID) {
	foreach (prioList hook,m_userDisconnectionReuestList)
		{
			if (hook.hook->hookUserDisconnectionRequest(queue, connID)
					== HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookRoutingRequest(bool onlyfree, source_t source,
		sink_t sink, QList<genRoute_t>* ReturnList) {
	foreach (prioList hook,m_routingRequestList)
		{
			if (hook.hook->hookRoutingRequest(onlyfree, source, sink,
					ReturnList) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

//todo change type
genError_t HookHandler::fireHookRoutingComplete(genRoute_t route) {
	foreach (prioList hook,m_routingCompleteList)
		{
			if (hook.hook->hookRoutingComplete(route) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookVolumeChange(volume_t newVolume, sink_t SinkID) {
	foreach (prioList hook,m_volumeChangeList)
		{
			if (hook.hook->hookVolumeChange(newVolume, SinkID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookMuteSource(source_t ID) {
	foreach (prioList hook,m_muteSourceList)
		{
			if (hook.hook->hookMuteSource(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookUnmuteSource(source_t ID) {
	foreach (prioList hook,m_unmuteSourceList)
		{
			if (hook.hook->hookUnmuteSource(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookMuteSink(sink_t ID) {
	foreach (prioList hook,m_muteSinkList)
		{
			if (hook.hook->hookMuteSink(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookUnmuteSink(sink_t ID) {
	foreach (prioList hook,m_unmuteSinkList)
		{
			if (hook.hook->hookUnmuteSink(ID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

genError_t HookHandler::fireHookInterruptRequest(Queue* queue,
		source_t interruptSource, sink_t sink, genInt_t* interruptID) {
	foreach (prioList hook,m_interruptRequestList)
		{
			if (hook.hook->hookInterruptRequest(queue, interruptSource, sink,
					interruptID) == HOOK_STOP)
				break;
		}
	return GEN_OK;
}

void HookHandler::registerAudioManagerCore(AudioManagerCore* core) {
	m_core = core;
}

void HookHandler::loadHookPlugins() {
	BaseHook *b = NULL;
	foreach (QObject *plugin, QPluginLoader::staticInstances())
		{
			HookPluginFactory* HookPluginFactory_ = qobject_cast<
					HookPluginFactory *> (plugin);
			if (HookPluginFactory_) {
				b = HookPluginFactory_->returnInstance();
				b->registerHookEngine(this);
				b->registerAudioManagerCore(m_core);
				b->InitHook();
				char pName[40];
				if (b->returnPluginName(pName) == GEN_OK) {
					DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Registered Hook Plugin:"), DLT_STRING(pName));
				}
			}
		}
}

