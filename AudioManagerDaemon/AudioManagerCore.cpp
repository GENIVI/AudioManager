/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file AudioManagerCore.cpp
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


#include "AudioManagerCore.h"
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>

Task::Task() {
}

Task::~Task() {
}

Task::Task(AudioManagerCore* core) {
	m_core = core;
}

TaskAsyncConnect::TaskAsyncConnect(AudioManagerCore* core, sink_t sink,
		source_t source) :
	Task(core), m_ParamSink(sink), m_ParamSource(source) {
}

TaskAsyncConnect::~TaskAsyncConnect() {
}

void TaskAsyncConnect::executeTask(Queue* queue) {
	/**
	 * \todo remove signals and slots, change to non QT timers
	 */
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
//	QObject::connect((const QObject*) m_core->returnReceiver(),
//			SIGNAL(signal_ackConnect(genHandle_t, genError_t)),
//			(const QObject*) this,
//			SLOT(slot_connect_finished(genHandle_t, genError_t)));
//	m_timer = new QTimer();
//	m_timer->setSingleShot(true);
//	QObject::connect(m_timer, SIGNAL(timeout()), (const QObject*) this,
//			SLOT(slot_timeout()));
//	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Started Connect"));
//	m_timer->start(CONNECT_TIMEOUT);
	m_core->connect(m_ParamSource, m_ParamSink, &m_handle);
}

void TaskAsyncConnect::setSink(sink_t sink) {
	m_ParamSink = sink;
}

void TaskAsyncConnect::setSource(source_t source) {
	m_ParamSource = source;
}

sink_t TaskAsyncConnect::getSink() {
	return m_ParamSink;
}

source_t TaskAsyncConnect::getSource() {
	return m_ParamSource;
}

void TaskAsyncConnect::slot_connect_finished(genHandle_t handle,
		genError_t error) {
//	if (handle == m_handle && error == GEN_OK) {
//		m_timer->stop();
//		delete m_timer;
//		m_timer = NULL;
//		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Connect returned"));
//		emit signal_nextTask();
//	} else {
//		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Somethings wrong with the connection"));
//	}
}

/**
 * \todo handle this event better.
 */
void TaskAsyncConnect::slot_timeout() {
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Connection timed out"));
}

TaskConnect::TaskConnect(AudioManagerCore* core, sink_t sink, source_t source) :
	Task(core), m_ParamSink(sink), m_ParamSource(source) {
}

TaskConnect::~TaskConnect() {
}

void TaskConnect::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
//	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Started Syncronous Connect"));
	m_core->connect(m_ParamSource, m_ParamSink);
	//emit signal_nextTask();
}

TaskDisconnect::TaskDisconnect(AudioManagerCore* core, connection_t ID) :
	Task(core), m_ParamConnectionID(ID) {
}

TaskDisconnect::~TaskDisconnect() {
}

void TaskDisconnect::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->disconnect(m_ParamConnectionID);
//	emit signal_nextTask();
}

TaskInterruptWait::TaskInterruptWait(AudioManagerCore* core,
		genInt_t interruptID) :
	Task(core), m_interruptID(interruptID) {
}

TaskInterruptWait::~TaskInterruptWait() {

}

void TaskInterruptWait::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
//	QObject::connect((const QObject*) m_core->returnCommandInterface(),
//			SIGNAL(signal_interruptResume(genInt_t)), (const QObject*) this,
//			SLOT(slot_interrupt_ready(genInt_t)));
}

void TaskInterruptWait::slot_interrupt_ready(genInt_t ID) {
	if (ID == m_interruptID) {
		//emit signal_nextTask();
	}
}

TaskSetVolume::TaskSetVolume(AudioManagerCore* core, volume_t newVolume,
		sink_t sink) :
	Task(core), m_volume(newVolume), m_sink(sink) {

}

TaskSetVolume::~TaskSetVolume() {
}

void TaskSetVolume::setVolume(volume_t newVolume) {
	m_volume = newVolume;
}

void TaskSetVolume::setSink(sink_t sink) {
	m_sink = sink;
}

volume_t TaskSetVolume::getVolume() {
	return m_volume;
}

sink_t TaskSetVolume::getSink() {
	return m_sink;
}

void TaskSetVolume::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
//	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Started Changed Volume"));
	m_core->setVolume(m_sink, m_volume);
//	emit signal_nextTask();
}

TaskSetSourceVolume::TaskSetSourceVolume(AudioManagerCore* core,
		volume_t newVolume, source_t source) :
	Task(core), m_volume(newVolume), m_source(source) {

}

TaskSetSourceVolume::~TaskSetSourceVolume() {

}

void TaskSetSourceVolume::setVolume(volume_t newVolume) {
	m_volume = newVolume;
}

void TaskSetSourceVolume::setSource(source_t source) {
	m_source = source;
}

volume_t TaskSetSourceVolume::getVolume() {
	return m_volume;
}
source_t TaskSetSourceVolume::getSource() {
	return m_source;
}

void TaskSetSourceVolume::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Started Changed Source Volume"));
	m_core->setSourceVolume(m_source, m_volume);
//	emit signal_nextTask();
}

TaskWait::TaskWait(AudioManagerCore* core, int mseconds) :
	Task(core), m_ParamMSeconds(mseconds) {
}

TaskWait::~TaskWait() {
}

void TaskWait::setTime(int mseconds) {
	m_ParamMSeconds = mseconds;
}

int TaskWait::getTime() {
	return m_ParamMSeconds;
}

void TaskWait::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
//	m_timer = new QTimer();
//	m_timer->setSingleShot(true);
//	QObject::connect(m_timer, SIGNAL(timeout()), (const QObject*) this,
//			SLOT(slot_timeIsup()));
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Start Sleep "));
//	m_timer->start(m_ParamMSeconds);
}

void TaskWait::slot_timeIsup() {
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Stop Sleep "));
//	m_timer->stop();
//	emit signal_nextTask();
}

TaskEnterUserConnect::TaskEnterUserConnect(AudioManagerCore* core,
		genRoute_t route, connection_t connID) :
	Task(core), m_route(route), m_connectionID(connID) {
}

TaskEnterUserConnect::~TaskEnterUserConnect() {
}

void TaskEnterUserConnect::setConnection(genRoute_t route, connection_t connID) {
	m_route = route;
	m_connectionID = connID;
}

genRoute_t TaskEnterUserConnect::returnConnection() {
	return m_route;
}

void TaskEnterUserConnect::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->returnDatabaseHandler()->updateMainConnection(m_connectionID,
			m_route);
//	emit signal_nextTask();
}

TaskRemoveUserConnect::TaskRemoveUserConnect(AudioManagerCore* core,
		connection_t connID) :
	Task(core), m_connectionID(connID) {
}

TaskRemoveUserConnect::~TaskRemoveUserConnect() {

}

void TaskRemoveUserConnect::setConnectionID(connection_t connID) {
	m_connectionID = connID;
}

connection_t TaskRemoveUserConnect::returnConnectionID() {
	return m_connectionID;
}

void TaskRemoveUserConnect::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->returnDatabaseHandler()->removeMainConnection(m_connectionID);
//	emit signal_nextTask();
}

TaskEnterInterrupt::TaskEnterInterrupt(AudioManagerCore* core, genInt_t ID,
		bool mixed, connection_t connID, std::list<source_t> listInterruptedSources) :
	Task(core), m_intID(ID), m_mixed(mixed), m_connectionID(connID),
			m_interruptedSourcesList(listInterruptedSources) {
}

TaskEnterInterrupt::~TaskEnterInterrupt() {
}

void TaskEnterInterrupt::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->returnDatabaseHandler()->updateInterrupt(m_intID, m_connectionID,
			m_mixed, m_interruptedSourcesList);
//	emit signal_nextTask();
}

TaskRemoveInterrupt::TaskRemoveInterrupt(AudioManagerCore* core, genInt_t ID) :
	Task(core), m_intID(ID) {
}

TaskRemoveInterrupt::~TaskRemoveInterrupt() {
}

void TaskRemoveInterrupt::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->returnDatabaseHandler()->removeInterrupt(m_intID);
//	emit signal_nextTask();
}

TaskSetSourceMute::TaskSetSourceMute(AudioManagerCore* core, source_t source) :
	Task(core), m_source(source) {
}

TaskSetSourceMute::~TaskSetSourceMute() {
}

void TaskSetSourceMute::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->setSourceMute(m_source);
//	emit signal_nextTask();
}

TaskSetSourceUnmute::TaskSetSourceUnmute(AudioManagerCore* core,
		source_t source) :
	Task(core), m_source(source) {
}

TaskSetSourceUnmute::~TaskSetSourceUnmute() {
}

void TaskSetSourceUnmute::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->setSourceUnMute(m_source);
//	emit signal_nextTask();
}

TaskEmitSignalConnect::TaskEmitSignalConnect(AudioManagerCore* core) :
	Task(core) {
}

TaskEmitSignalConnect::~TaskEmitSignalConnect() {
}

void TaskEmitSignalConnect::executeTask(Queue* queue) {
//	QObject::connect((const QObject*) this, SIGNAL(signal_nextTask()),
//			(const QObject*) queue, SLOT(slot_nextTask()));
	m_core->emitSignalConnect();
//	emit signal_nextTask();
}

Queue::Queue(AudioManagerCore* core, std::string name) :
	m_core(core), m_currentIndex(0), m_name(name), m_taskList(std::list<Task*> ()) {
	m_core->addQueue(this);
}

Queue::~Queue() {
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("shoot all tasks"));
//	foreach (Task* task, m_taskList) {
//		delete task;
//	}
	m_core->removeQueue(this);
}

void Queue::run() {
	if (m_taskList.empty() == false) {
		Task* task = m_taskList.front();
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Started to execute Task"));
		task->executeTask(this);
	} else {
		DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("empty tasklist was attempted to run "));
	}

}

void Queue::slot_nextTask() {
	m_currentIndex++;
//	if (m_currentIndex < m_taskList.size()) {
//		//m_currentIndex->executeTask(*this);
//	} else {
//		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Finished all Tasks"));
//		this->~Queue();
//	}
}

void Queue::addTask(Task* task) {
	m_taskList.push_back(task);
}

genError_t Queue::removeTask(Task* task) {
//	if (m_taskList.remove(task) > 0) {
//		return GEN_OK;
//	}
	return GEN_OUTOFRANGE;
	void addQueue(Queue* queue);

}

std::list<Task*> Queue::getTaskList() {
	return m_taskList;
}

std::string Queue::returnName() {
	return m_name;
}

AudioManagerCore::AudioManagerCore() :
	m_queueList(std::list<Queue*> ()) {
}

AudioManagerCore::~AudioManagerCore() {
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("delete all running queues"));
	std::list<Queue*>::iterator qIterator;
	for(qIterator=m_queueList.begin();qIterator!=m_queueList.end();qIterator++) {
		delete *qIterator;
	}
}

Router* AudioManagerCore::returnRouter() {
	return m_router;
}

DataBaseHandler* AudioManagerCore::returnDatabaseHandler() {
	return m_databaseHandler;
}

RoutingReceiver* AudioManagerCore::returnReceiver() {
	return m_receiver;
}

CommandInterface* AudioManagerCore::returnCommandInterface() {
	return m_command;
}

void AudioManagerCore::registerBushandler(Bushandler* handler) {
	m_busHandler = handler;
}

void AudioManagerCore::registerDatabasehandler(DataBaseHandler* handler) {
	m_databaseHandler = handler;
}

void AudioManagerCore::registerRouter(Router* router) {
	m_router = router;
}

void AudioManagerCore::registerHookEngine(HookHandler* handler) {
	m_hookHandler = handler;
}

void AudioManagerCore::registerReceiver(RoutingReceiver* receiver) {
	m_receiver = receiver;
}

void AudioManagerCore::registerCommandInterface(CommandInterface* command) {
	m_command = command;
}

genError_t AudioManagerCore::UserConnect(source_t source, sink_t sink) {
	Queue* userConnectionRequestQ = new Queue(this, "User Connect");
	m_hookHandler->fireHookUserConnectionRequest(userConnectionRequestQ,
			source, sink);
	userConnectionRequestQ->run();
	return GEN_OK;
}

genError_t AudioManagerCore::UserDisconnect(connection_t connID) {
	Queue* userDisconnectionRequestQ = new Queue(this, "User Disconnect");
	m_hookHandler->fireHookUserDisconnectionRequest(userDisconnectionRequestQ,
			connID);
	userDisconnectionRequestQ->run();
	return GEN_OK;
}

genError_t AudioManagerCore::UserSetVolume(sink_t sink, volume_t volume) {
	m_hookHandler->fireHookVolumeChange(volume, sink);
	return GEN_OK;
}

std::list<ConnectionType> AudioManagerCore::getListConnections() {
	return m_databaseHandler->getListAllMainConnections();
}

std::list<SinkType> AudioManagerCore::getListSinks() {
	std::list<SinkType> sinkList;
	m_databaseHandler->getListofSinks(&sinkList);
	return sinkList;
}

std::list<SourceType> AudioManagerCore::getListSources() {
	std::list<SourceType> sourceList;
	m_databaseHandler->getListofSources(&sourceList);
	return sourceList;
}

void AudioManagerCore::emitSignalConnect() {
//	emit signal_connecionChanged();
}

genError_t AudioManagerCore::connect(source_t source, sink_t sink,
		genHandle_t* handle) {
	genHandle_t localhandle;
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Connect"), DLT_INT(source), DLT_STRING(" to "), DLT_INT(sink));
	domain_t domainID = m_databaseHandler->get_Domain_ID_from_Source_ID(source);
	RoutingSendInterface* iface = m_busHandler->getInterfaceforBus(
			m_databaseHandler->get_Bus_from_Domain_ID(domainID));
	localhandle = m_databaseHandler->insertConnection(source, sink);

	if (handle) {
		*handle = localhandle;
	}
	iface->connect(source, sink, localhandle);
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Connect finished"));
	return GEN_OK;
	/**
	 * \todo implement error handling
	 */
}

genError_t AudioManagerCore::disconnect(connection_t ID) {
	domain_t domainID = m_databaseHandler->get_Domain_ID_from_Connection_ID(ID);
	RoutingSendInterface* iface = m_busHandler->getInterfaceforBus(
			m_databaseHandler->get_Bus_from_Domain_ID(domainID));
	iface->disconnect(ID);
	m_databaseHandler->removeConnection(ID);
	return GEN_OK;
	/**
	 * \todo failure handling
	 */
}

genError_t AudioManagerCore::setVolume(sink_t sink, volume_t volume) {
	RoutingSendInterface* iface = m_busHandler->getInterfaceforBus(
			m_databaseHandler->get_Bus_from_Domain_ID(
					m_databaseHandler->get_Domain_ID_from_Sink_ID(sink)));
	iface->setSinkVolume(volume, sink);
	return GEN_OK;
	/**
	 * \todo failure handling
	 */
}

genError_t AudioManagerCore::interruptRequest(source_t interruptSource,
		sink_t sink, genInt_t* interrupt) {
	Queue* interruptRequestQ = new Queue(this, "Interrupt Request");
	m_hookHandler->fireHookInterruptRequest(interruptRequestQ, interruptSource,
			sink, interrupt);
	interruptRequestQ->run();
	return GEN_OK;
}

genError_t AudioManagerCore::setSourceVolume(source_t source, volume_t volume) {
	RoutingSendInterface* iface = m_busHandler->getInterfaceforBus(
			m_databaseHandler->get_Bus_from_Domain_ID(
					m_databaseHandler->get_Domain_ID_from_Source_ID(source)));
	iface->setSourceVolume(volume, source);
	return GEN_OK;
}

genError_t AudioManagerCore::setSourceMute(source_t source) {
	RoutingSendInterface* iface = m_busHandler->getInterfaceforBus(
			m_databaseHandler->get_Bus_from_Domain_ID(
					m_databaseHandler->get_Domain_ID_from_Source_ID(source)));
	iface->muteSource(source);
	return GEN_OK;
}

genError_t AudioManagerCore::setSourceUnMute(source_t source) {
	RoutingSendInterface* iface = m_busHandler->getInterfaceforBus(
			m_databaseHandler->get_Bus_from_Domain_ID(
					m_databaseHandler->get_Domain_ID_from_Source_ID(source)));
	iface->unmuteSource(source);
	return GEN_OK;
}

genError_t AudioManagerCore::getRoute(const bool onlyfree,
		const source_t source, const sink_t sink, std::list<genRoute_t>* ReturnList) {
	m_hookHandler->fireHookRoutingRequest(onlyfree, source, sink, ReturnList);
	return GEN_OK;
}

connection_t AudioManagerCore::returnConnectionIDforSinkSource(sink_t sink,
		source_t source) {
	return m_databaseHandler->getConnectionID(source, sink);
}

genError_t AudioManagerCore::removeQueue(Queue* queue) {
//	if (m_queueList.remove(*queue)) {
//		return GEN_OK;
//	} else {
//		return GEN_OUTOFRANGE;
//	}
}

source_t AudioManagerCore::returnSourceIDfromName(const std::string name) {
	return m_databaseHandler->get_Source_ID_from_Name(name);
}

sink_t AudioManagerCore::returnSinkIDfromName(const std::string name) {
	return m_databaseHandler->get_Sink_ID_from_Name(name);
}

void AudioManagerCore::addQueue(Queue* queue) {
	m_queueList.push_back(queue);
}

std::list<std::string> AudioManagerCore::getSharedLibrariesFromDirectory(std::string dirName) {

	std::list<std::string> fileList;
	// open directory
    DIR *directory = opendir(dirName.c_str());
    if (!directory)
    {
    	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Error opening directory "),DLT_STRING(dirName.c_str()));
        return fileList;
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
        	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("PluginSearch adding file "),DLT_STRING(entryName.c_str()));
            fileList.push_back(dirName + "/" + entryName);
        }
        else
        {
        	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("PluginSearch ignoring file "),DLT_STRING(entryName.c_str()));
        }
    }

    closedir(directory);
    return fileList;
}


