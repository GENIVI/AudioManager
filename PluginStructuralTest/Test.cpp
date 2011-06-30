/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * PluginTest
 *
 * \file Test.cpp
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
 */

#include "Test.h"

TestPlugin::TestPlugin() {}

TestPlugin::~TestPlugin() {}

genError_t TestPlugin::InitHook(void) {
	/**
	 * This is the right place for init stuff that needs to be done. Make sure that all the  Hooks are registered here !
	 */
	m_hookhandler->registerHook(10,HOOK_DOMAIN_REGISTER,this);
	m_hookhandler->registerHook(1,HOOK_SINK_REGISTER,this);
	m_hookhandler->registerHook(1,HOOK_USER_CONNECTION_REQUEST,this);
	m_hookhandler->registerHook(1,HOOK_USER_DISCONNECTION_REQUEST,this);
	m_hookhandler->registerHook(1,HOOK_ROUTING_REQUEST,this);
	m_hookhandler->registerHook(1,HOOK_INTERRUPT_REQUEST,this);
	m_hookhandler->registerHook(1,HOOK_VOLUME_CHANGE,this);

	return GEN_OK;
}

genError_t TestPlugin::returnPluginName(char* name) {
	strcpy(name,"Test Plugin");
	return GEN_OK;
}

genError_t TestPlugin::DeinitHook(void) {
	return GEN_OK;

}

genHookResult_t TestPlugin::hookDomainRegister (char* Name, domain_t ID) {
	(void)Name;
	(void)ID;
	return HOOK_OK;
}

genHookResult_t TestPlugin::hookSinkRegister(char* Name, sink_t ID) {
	(void)Name;
	(void)ID;
	return HOOK_OK;
}

genHookResult_t TestPlugin::hookUserConnectionRequest (Queue* queue, source_t SourceID, sink_t SinkID) {
	DataBaseHandler* handler=m_core->returnDatabaseHandler();
	genRoute_t route;
	QList<genRoutingElement_t> list;

	source_t domainSourceID = handler->get_Domain_ID_from_Source_ID(SourceID);
	sink_t domainSinkID = handler->get_Domain_ID_from_Sink_ID(SinkID);
	connection_t connID=handler->reserveMainConnection(SourceID,SinkID);

	//first go for the obvious:
	if (domainSinkID == domainSourceID) {
		genRoutingElement_t r;
		r.sink = SinkID;
		r.source = SourceID;
		r.Domain_ID = handler->get_Domain_ID_from_Source_ID(SourceID);
		list.append(r);
		route.Sink_ID = SinkID;
		route.Source_ID = SourceID;
		route.route = list;
	} else {
		QList<genRoute_t> listofRoutes;
		m_core->getRoute(false,SourceID,SinkID,&listofRoutes);

		if (listofRoutes.length() > 0) {
			list = listofRoutes.first().route; //TODO these are all routes. Currently we take only the first
			route= listofRoutes.first();
		}
	}

	foreach (genRoutingElement_t element, list) {
			TaskConnect* connect=new TaskConnect(m_core, element.sink, element.source);
			queue->addTask(connect);
	}

	TaskEnterUserConnect* enterConnect=new TaskEnterUserConnect(m_core, route, connID);
	queue->addTask(enterConnect);

	TaskEmitSignalConnect* emitConnect=new TaskEmitSignalConnect(m_core);
	queue->addTask(emitConnect);

	return HOOK_OK;
}

genHookResult_t TestPlugin::hookUserDisconnectionRequest  (Queue* queue,  connection_t connID) {

	genRoute_t* route;
	sink_t sink;
	source_t source;
	m_core->returnDatabaseHandler()->getMainConnectionDatafromID(connID,&sink,&source,&route);
	foreach (genRoutingElement_t routingElement, route->route) {
		connection_t connectionID = m_core->returnConnectionIDforSinkSource (routingElement.sink, routingElement.source);
		TaskDisconnect* disconnect=new TaskDisconnect(m_core,connectionID);
		queue->addTask(disconnect);
	}
	TaskRemoveUserConnect* userDisconnect=new TaskRemoveUserConnect(m_core,connID);
	queue->addTask(userDisconnect);

	TaskEmitSignalConnect* emitConnect=new TaskEmitSignalConnect(m_core);
	queue->addTask(emitConnect);

	return HOOK_OK;
}

genHookResult_t TestPlugin::hookVolumeChange (volume_t newVolume, sink_t SinkID) {
	Queue* volumeChange = new Queue(m_core,"Volume Change");
	TaskSetVolume* volumeTask = new TaskSetVolume(m_core,newVolume,SinkID);
	volumeChange->addTask(volumeTask);
	volumeChange->run();
	return HOOK_OK;
}

genHookResult_t TestPlugin::hookRoutingRequest (bool onlyfree,source_t source, sink_t sink,QList<genRoute_t>* ReturnList) {
	//Here could be a place to modify, the request, take care of restrictions etc...
	Router* router=m_core->returnRouter();
	router->get_Route_from_Source_ID_to_Sink_ID(onlyfree,source,sink,ReturnList);
	return HOOK_OK;
}

genHookResult_t TestPlugin::hookInterruptRequest (Queue* queue, source_t interruptSource, sink_t sink, genInt_t* interruptID) {
	interruptType_t interrupt;
	interrupt.sourceID=interruptSource;
	interrupt.SinkID=sink;
	interrupt.ID=m_core->returnDatabaseHandler()->reserveInterrupt(sink,interruptSource);
	*interruptID=interrupt.ID;
	interrupt.listInterrruptedSources=m_core->returnDatabaseHandler()->getSourceIDsForSinkID(sink);
	interrupt.connID=m_core->returnDatabaseHandler()->reserveMainConnection(interruptSource,sink);
	interrupt.mixed=m_core->returnDatabaseHandler()->is_source_Mixed(interruptSource);

	if (interrupt.mixed) {
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Add task to change the volume on interrupt"));
		foreach(source_t sourceL,interrupt.listInterrruptedSources) {
			DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Set volume change for source"),DLT_INT(sourceL));
			TaskSetSourceVolume* volumetask=new TaskSetSourceVolume(m_core,INTERRUPT_VOLUME_LEVEL,sourceL);
			queue->addTask(volumetask);
		}
	} else {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Add task to switch off sources on interrupt"));
		foreach(source_t sourceL,interrupt.listInterrruptedSources) {
			TaskSetSourceMute* mutetask=new TaskSetSourceMute(m_core,sourceL);
			queue->addTask(mutetask);
		}
	}
	genRoute_t route;
	QList<genRoutingElement_t> list;

	source_t domainSourceID = m_core->returnDatabaseHandler()->get_Domain_ID_from_Source_ID(interruptSource);
	sink_t domainSinkID = m_core->returnDatabaseHandler()->get_Domain_ID_from_Sink_ID(sink);

	//first go for the obvious:
	if (domainSinkID == domainSourceID) {
		genRoutingElement_t r;
		r.sink = sink;
		r.source = interruptSource;
		r.Domain_ID = domainSourceID;
		list.append(r);
		route.Sink_ID = sink;
		route.Source_ID = interruptSource;
		route.len = 1;
		route.route = list;
	} else {
		QList<genRoute_t> listofRoutes;
		m_core->getRoute(false,interruptSource,sink,&listofRoutes);

		if (listofRoutes.length() > 0) {
			list = listofRoutes.first().route; //TODO these are all routes. Currently we take only the first
			route= listofRoutes.first();
		}
	}

	foreach (genRoutingElement_t element, list) {
			TaskConnect* connect=new TaskConnect(m_core, element.sink, element.source);
			queue->addTask(connect);
	}

	TaskEnterUserConnect* enterConnect=new TaskEnterUserConnect(m_core,route,interrupt.connID);
	queue->addTask(enterConnect);

	TaskEnterInterrupt* enterInteruppt=new TaskEnterInterrupt(m_core,interrupt.ID,m_core->returnDatabaseHandler()->is_source_Mixed(interruptSource),interrupt.connID,interrupt.listInterrruptedSources);
	queue->addTask(enterInteruppt);

	TaskInterruptWait* interruptWait=new TaskInterruptWait(m_core,interrupt.ID);
	queue->addTask(interruptWait);

	TaskEmitSignalConnect* emitConnect2=new TaskEmitSignalConnect(m_core);
	queue->addTask(emitConnect2);

	foreach (genRoutingElement_t element, list) {
			TaskDisconnect* disconnect=new TaskDisconnect(m_core,m_core->returnDatabaseHandler()->getConnectionID(element.source,element.sink));
			queue->addTask(disconnect);
	}
	if (interrupt.mixed) {
		foreach(source_t sourceL,interrupt.listInterrruptedSources) {
			TaskSetSourceVolume* volumetask=new TaskSetSourceVolume(m_core,NORMAL_VOLUME_LEVEL,sourceL);
			queue->addTask(volumetask);
		}
	} else {
		foreach(source_t sourceL,interrupt.listInterrruptedSources) {
			TaskSetSourceUnmute* unmutetask=new TaskSetSourceUnmute(m_core,sourceL);
			queue->addTask(unmutetask);
		}
	}

	TaskRemoveUserConnect* removeConnect=new TaskRemoveUserConnect(m_core,interrupt.connID);
	queue->addTask(removeConnect);

	TaskRemoveInterrupt* removeInterrupt=new TaskRemoveInterrupt(m_core,interrupt.ID);
	queue->addTask(removeInterrupt);

	TaskEmitSignalConnect* emitConnect=new TaskEmitSignalConnect(m_core);
	queue->addTask(emitConnect);

	return HOOK_OK;
}

BaseHook* TestHookPluginFactory::returnInstance() {
	return new TestPlugin;
}

Q_EXPORT_PLUGIN2(TestPlugin, TestHookPluginFactory);

