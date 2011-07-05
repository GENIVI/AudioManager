/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file AudioManagerCore.h
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

#ifndef AUDIOMANAGERCORE_H_
#define AUDIOMANAGERCORE_H_

#include <QtCore/QObject>

#include "audioManagerIncludes.h"

/**CONSTANT for the Connection timeout when using asynchronous connect. In mseconds.
 *
 */
#define CONNECT_TIMEOUT 3000

class HookHandler;
class AudioManagerCore;
class Queue;
class Router;
class Bushandler;
class DBusCommandInterface;

/**
 * \class Task
 * \brief Base Class for all Tasks
 * \details a Base Class for a basic operation together with a set of parameters
 * Every Task that can be created by the Hook Plugins is derived from this class
 * Be sure to implement executeTask() because this will be called when the task is executed
 * Whenever the task is finished, you need to emit the signal signal_nextTask() that will call the next
 * Task in the queue.
 * \fn virtual void Task::executeTask(Queue* queue)=0
 * \brief This needs to be overwritten in implementation of the class
 * The queue calls this function to start the execution of the task
 * \param queue pointer to the queue
 * \fn void Task::signal_nextTask()
 * \brief by emitting this signal, the next task in the queue is triggered
 */
class Task: public QObject {
Q_OBJECT
public:
	Task();
	Task(AudioManagerCore* core);
	virtual ~Task();
	virtual void executeTask(Queue* queue)=0;

signals:
	void signal_nextTask();
protected:
	AudioManagerCore* m_core; //!< pointer to the core
};

/**
 * \class TaskAsyncConnect
 * \brief This task is used to connect a source and a sink asynchronous
 *
 * \fn TaskAsyncConnect::TaskAsyncConnect(AudioManagerCore* core, sink_t sink, source_t source)
 * \brief Constructor
 * \param core pointer to the core
 * \param sink	the sink to be connected
 * \param source the source to be connected
 *
 * \fn void TaskAsyncConnect::setSink(sink_t sink)
 * \brief Sets the Sink that should be connected to
 * \param sink the sink
 *
 * \fn void TaskAsyncConnect::setSource(source_t source)
 * \brief sets the source
 * \param source the source
 *
 * \fn sink_t TaskAsyncConnect::getSink()
 * \brief returns the sink
 *
 * \fn source_t TaskAsyncConnect::getSource()
 * \brief returns the source
 *
 * \fn void TaskAsyncConnect::slot_connect_finished(genHandle_t handle, genError_t error)
 * \brief is used to receive the asynchronous answer.
 *
 * \fn void TaskAsyncConnect::slot_timeout()
 * \brief used to handle the timeout
 */
class TaskAsyncConnect: public Task {
Q_OBJECT
public:
	TaskAsyncConnect(AudioManagerCore* core, sink_t sink, source_t source);
	virtual ~TaskAsyncConnect();
	void setSink(sink_t sink);
	void setSource(source_t source);
	sink_t getSink();
	source_t getSource();
	void executeTask(Queue* queue);

public slots:
	void slot_connect_finished(genHandle_t handle, genError_t error);
	void slot_timeout();
private:
	sink_t m_ParamSink; //!< the sink to be connected
	source_t m_ParamSource; //!< the source to be connected
	genHandle_t m_handle; //!< the handle (ConnectionID)
	QTimer* m_timer; //!< pointer to a timer used for timeout tracking
};

/**
 * \class TaskConnect
 * \brief This task is used to connect a source and a sink synchronous
 *
 * \fn TaskConnect::TaskConnect(AudioManagerCore* core, sink_t sink, source_t source)
 * \brief constructor
 * \param core pointer to the core
 * \param sink	the sink to be connected
 * \param source the source to be connected
 *
 */
class TaskConnect: public Task {
Q_OBJECT
public:
	TaskConnect(AudioManagerCore* core, sink_t sink, source_t source);
	virtual ~TaskConnect();
	void executeTask(Queue* queue);

private:
	sink_t m_ParamSink; //!< the sink to be connected
	source_t m_ParamSource; //!< the source to be connected
};

/**
 * \class TaskDisconnect
 * \brief This task is used to connect a source and a sink synchronous
 *
 * \fn TaskDisconnect::TaskDisconnect(AudioManagerCore* core, connection_t ID)
 * \brief constructor
 * \param core pointer to the core
 * \param ID the connection ID to be disconnected
 *
 */
class TaskDisconnect: public Task {
Q_OBJECT
public:
	TaskDisconnect(AudioManagerCore* core, connection_t ID);
	virtual ~TaskDisconnect();
	void executeTask(Queue* queue);
private:
	connection_t m_ParamConnectionID; //!< the connection to disconnect
};

/**
 * \class TaskInterruptWait
 * \brief this class waits for an interrupt to be resumed
 *
 * \fn TaskInterruptWait::TaskInterruptWait(AudioManagerCore* core, genInt_t connectionID)
 * \brief Constructor
 * \param core link to AudioManagercore
 * \param connectionID the connection ID to be waited for
 *
 * \fn void TaskInterruptWait::slot_interrupt_ready(genInt_t ID)
 * \brief slot that is called when the interrupt resumes
 * \param ID the interrupt ID of the interrupt that resumed
 */
class TaskInterruptWait: public Task {
Q_OBJECT
public:
	TaskInterruptWait(AudioManagerCore* core, genInt_t connectionID);
	virtual ~TaskInterruptWait();
	void executeTask(Queue* queue);
public slots:
	void slot_interrupt_ready(genInt_t ID);
private:
	genInt_t m_interruptID; //!< the interrupt ID that should be waited for
};

/**
 * \class TaskSetVolume
 * \brief a task that sets the volume for a sink
 *
 * \fn TaskSetVolume::TaskSetVolume(AudioManagerCore* core, volume_t newVolume=0, sink_t sink=0)
 * \brief constructor
 * \param core the pointer to the AudioManagerCore
 * \param newVolume the volume to be set
 * \param sink the sink that the volume shall be applied to
 *
 * \fn void TaskSetVolume::setVolume(volume_t newVolume)
 * \brief sets the volume
 *
 * \fn void TaskSetVolume::setSink(sink_t sink)
 * \brief sets the sink
 *
 * \fn volume_t  TaskSetVolume::getVolume()
 * \brief returns the volume
 *
 * \fn sink_t TaskSetVolume::getSink()
 * \brief returns the volume
 */
class TaskSetVolume: public Task {
Q_OBJECT
public:
	TaskSetVolume(AudioManagerCore* core, volume_t newVolume=0, sink_t sink=0);
	virtual ~TaskSetVolume();

	void setVolume(volume_t newVolume);
	void setSink(sink_t sink);

	volume_t getVolume();
	sink_t getSink();

	void executeTask(Queue* queue);
private:
	volume_t m_volume; //!< the volume
	sink_t m_sink; //!< the sink
};

/**
 * \class TaskSetSourceVolume
 * \brief sets the volume of a source
 *
 * \fn TaskSetSourceVolume::TaskSetSourceVolume(AudioManagerCore* core, volume_t newVolume=0, source_t source=0)
 * \brief constructor
 * \param core the pointer to the AudioManagerCore
 * \param newVolume the volumesink_t  to be set
 * \param source the source that the volume shall be applied to
 *
 *  * \fn void TaskSetSourceVolume::setVolume(volume_t newVolume)
 * \brief sets the volume
 *
 * \fn void TaskSetSourceVolume::setSource (source_t source)
 * \brief sets the sink
 *
 * \fn volume_t TaskSetSourceVolume::getVolume()
 * \brief returns the volume
 */
class TaskSetSourceVolume: public Task {
Q_OBJECT
public:
	TaskSetSourceVolume(AudioManagerCore* core, volume_t newVolume=0, source_t source=0);
	virtual ~TaskSetSourceVolume();

	void setVolume(volume_t newVolume);
	void setSource(source_t source);

	volume_t getVolume();
	source_t getSource();

	void executeTask(Queue* queue);
private:
	volume_t m_volume; //!< the volume
	source_t m_source; //!< the source
};

/**
 * \class TaskWait
 * \brief implements a wait in msecons
 *
 * \fn TaskWait::TaskWait(AudioManagerCore* core, int mseconds)
 * \brief constructor
 * \param core the pointer to the AudioManagerCore
 * \param mseconds delay in milliseconds
 *
 * \fn void TaskWait::setTime(int mseconds)
 * \brief sets the time
 *
 * \fn int TaskWait::getTime()
 * \brief returns the time
 *
 * \fn 	void TaskWait::slot_timeIsup()
 * \brief slot called when the time is up
 */
class TaskWait: public Task {
Q_OBJECT
public:
	TaskWait(AudioManagerCore* core, int mseconds);
	virtual ~TaskWait();
	void setTime(int mseconds);
	int getTime();
	void executeTask(Queue* queue);

public slots:
	void slot_timeIsup();
private:
	int m_ParamMSeconds;//!< time to be delayed in milli seconds
	QTimer* m_timer; //!< pointer to timer
};

/**
 * \class TaskEnterUserConnect
 * \brief This task enters the user connection into the struct of the AudioManagerCore
 * \details We need this because the connection shall be entered not before it is build up
 *
 * \fn TaskEnterUserConnect::TaskEnterUserConnect(AudioManagerCore* core, genRoute_t route, connection_t connID=0)
 * \brief enters a user connect into the database
 */
class TaskEnterUserConnect: public Task {
Q_OBJECT
public:
	TaskEnterUserConnect(AudioManagerCore* core, genRoute_t route, connection_t connID=0);
	virtual ~TaskEnterUserConnect();
	void setConnection(genRoute_t route, connection_t connID);
	genRoute_t returnConnection();
	void executeTask(Queue* queue);
private:
	genRoute_t m_route;		//!< the route to be entered
	connection_t m_connectionID; //!< the connection ID
};

/**
 * \class TaskRemoveUserConnect
 * \brief removes a user Connect
 *
 * \fn TaskRemoveUserConnect::TaskRemoveUserConnect(AudioManagerCore* core, connection_t connID)
 * \brief constructor
 */
class TaskRemoveUserConnect : public Task {
Q_OBJECT
public:
	TaskRemoveUserConnect(AudioManagerCore* core, connection_t connID);
	virtual ~TaskRemoveUserConnect();
	void setConnectionID(connection_t connID);
	connection_t returnConnectionID();
	void executeTask(Queue* queue);
private:
	connection_t m_connectionID; //!< the connection ID
};

/**
 * \class TaskEnterInterrupt
 * \brief enters an interrupt into the database
 */
class TaskEnterInterrupt: public Task {
Q_OBJECT
public:
	TaskEnterInterrupt(AudioManagerCore* core, genInt_t ID,bool mixed, connection_t connID,	QList<source_t> listInterruptedSources);
	virtual ~TaskEnterInterrupt();
	void executeTask(Queue* queue);
private:
	genInt_t m_intID; //!< the interrupt ID
	bool m_mixed; //!< true if mixed
	connection_t m_connectionID; //!< the connection ID
	QList<source_t> m_interruptedSourcesList; //!< the list of interrupted sources
};

/**
 * \class TaskRemoveInterrupt
 * \brief removes an interrupt
 */
class TaskRemoveInterrupt: public Task {
Q_OBJECT
public:
	TaskRemoveInterrupt(AudioManagerCore* core, genInt_t ID);
	virtual ~TaskRemoveInterrupt();
	void executeTask(Queue* queue);
private:
	genInt_t m_intID; //!< the interrupt ID
};

/**
 * \class TaskSetSourceMute
 * \brief mutes a source
 */
class TaskSetSourceMute: public Task {
Q_OBJECT
public:
	TaskSetSourceMute(AudioManagerCore* core, source_t source);
	virtual ~TaskSetSourceMute();
	void executeTask(Queue* queue);
private:
	source_t m_source; //!< the source to be muted
};

/**
 * \class TaskSetSourceUnmute
 * \brief unmutes a source
 */
class TaskSetSourceUnmute: public Task {
Q_OBJECT
public:
	TaskSetSourceUnmute(AudioManagerCore* core, source_t source);
	virtual ~TaskSetSourceUnmute();
	void executeTask(Queue* queue);
private:
	source_t m_source; //!< the source to be unmuted
};

/**
 * \class TaskEmitSignalConnect
 * \brief emits the signal connect
 */
class TaskEmitSignalConnect: public Task {
Q_OBJECT
public:
	TaskEmitSignalConnect(AudioManagerCore* core);
	virtual ~TaskEmitSignalConnect();
	void executeTask(Queue* queue);
};

/**
 * \class Queue
 * \brief With the help of this class, all events that need to be processed are organized and queued
 * \details tasks to be added have to be created with new, after the queue is done all tasks will be deleted and the memory freed.
 * \todo error handling
 * \todo sorting of tasks
 * \todo add some function to attach parameters to the Queue so that organizing and handling is possible
 *
 * \fn Queue::Queue(AudioManagerCore* core,QString name="")
 * \brief constructor
 * \param name give the queue a name
 * \param core pointer to AudioManagerCore
 *
 * \fn void Queue::run()
 * \brief starts the queue
 *
 * \fn void Queue::addTask(Task* task)
 * \brief add a task to the queue
 * \param task pointer to the task
 *
 * \fn genError_t Queue::removeTask(Task* task)
 * \brief removes a task
 * \param task pointer to the task
 * \return GEN_OK on success
 *
 * \fn 	QList<Task*> Queue::getTaskList()
 * \brief returns the actual task list
 * \return list with pointers to the tasks
 *
 * \fn QString Queue::returnName()
 * \brief returns the name of the Queue
 * \return the name in QString format
 *
 * \fn void Queue::slot_nextTask()
 * \brief is called when a task is finished and the next task can be called
 */
class Queue: public QObject {
Q_OBJECT
public:
	Queue(AudioManagerCore* core,QString name="");
	virtual ~Queue();
	void run();
	void addTask(Task* task);
	genError_t removeTask(Task* task);
	QList<Task*> getTaskList();
	QString returnName();

public slots:
	void slot_nextTask();
private:
	AudioManagerCore* m_core; //!< pointer to AudioManagerCore
	int m_currentIndex; //!< the index of the list wich is currently worked on
	QString m_name; //!< name of the Queue
	QList<Task*> m_taskList; //!< the list with pointers to the tasks
};

/**
 * \class AudioManagerCore
 * \brief The central Managing Class of the AudioManager
 *
 * \fn genError_t AudioManagerCore::UserConnect(source_t source, sink_t sink)
 * \brief does a user connect
 *
 * \fn genError_t AudioManagerCore::UserDisconnect(connection_t connID)
 * \brief does a user disconnect
 *
 * \fn genError_t AudioManagerCore::UserSetVolume(sink_t sink, volume_t volume)
 * \brief set the user volume (on sink level)
 *
 * \fn genError_t AudioManagerCore::connect(source_t source, sink_t sink, genHandle_t* handle = NULL)
 * \brief connects sources and sinks
 *
 * \fn genError_t AudioManagerCore::disconnect(connection_t ID)
 * \brief disconnects sources and sinks
 *
 * \fn genError_t AudioManagerCore::setVolume(sink_t sink, volume_t volume)
 * \brief sets the volume to a sink
 *
 * \fn genError_t AudioManagerCore::interruptRequest(source_t interruptSource, sink_t sink, genInt_t* interruptID)
 * \brief via this call, an interrupt is requested
 *
 * \fn genError_t AudioManagerCore::setSourceVolume (source_t source, volume_t volume)
 * \brief sets the source volume
 *
 * \fn genError_t AudioManagerCore::setSourceMute (source_t source)
 * \brief mutes a source
 *
 * \fn genError_t AudioManagerCore::setSourceUnMute (source_t source)
 * \brief unmutes a source
 *
 * \fn genError_t AudioManagerCore::getRoute(const bool onlyfree, const source_t source, const sink_t sink, QList<genRoute_t>* ReturnList)
 * \brief returns a route
 *
 * \fn connection_t AudioManagerCore::returnConnectionIDforSinkSource (sink_t sink, source_t source)
 * \brief returns the connection ID for a sink source combination
 *
 * \fn source_t AudioManagerCore::returnSourceIDfromName(const QString name)
 * \brief returns the source ID for a name
 *
 * \fn sink_t AudioManagerCore::returnSinkIDfromName(const QString name)
 * \brief returns the sink ID for a given name
 *
 * \fn QList<ConnectionType> AudioManagerCore::getListConnections()
 * \brief returns the list of connections
 *
 * \fn QList<SinkType> AudioManagerCore::getListSinks()
 * \brief returns a list of all sinks
 *
 * \fn QList<SourceType> AudioManagerCore::getListSources()
 * \brief returns a list of all sources
 *
 * \fn void AudioManagerCore::emitSignalConnect()
 * \brief emits the signal connect
 *
 * \fn Router* AudioManagerCore::returnRouter()
 * \brief returns the pointer to the router
 *
 * \fn DataBaseHandler* AudioManagerCore::returnDatabaseHandler()
 * \brief returns the pointer to the database handler
 *
 * \fn RoutingReceiver* AudioManagerCore::returnReceiver()
 * \brief returns the pointer to the receiver
 *
 * \fn DBusCommandInterface* AudioManagerCore::returnCommandInterface()
 * \brief returns the pointer to the command interface
 *
 * \fn void AudioManagerCore::registerDatabasehandler(DataBaseHandler * handler)
 * \brief registers the database handler @ the core
 *
 * \fn void AudioManagerCore::registerRouter(Router* router)
 * \brief registers the router @ the core
 *
 * \fn void AudioManagerCore::registerBushandler(Bushandler* handler)
 * \brief registers the bushandler @ the core
 *
 * \fn void AudioManagerCore::registerHookEngine(HookHandler* handler)
 * \brief registers the hook engine @ the core
 *
 * \fn void AudioManagerCore::registerReceiver(RoutingReceiver* receiver)
 * \brief registers the receiver @ the core
 *
 * \fn void AudioManagerCore::registerCommandInterface(DBusCommandInterface* command)
 * \brief registers the command interface @ the core
 *
 * \fn void AudioManagerCore::addQueue(Queue* queue)
 * \brief adds a queue
 *
 * \fn genError_t AudioManagerCore::removeQueue(Queue* queue)
 * \brief removes a queue
 *
 *
 */
class AudioManagerCore :public QObject {
Q_OBJECT
public:
	AudioManagerCore();
	virtual ~AudioManagerCore();

	genError_t UserConnect(source_t source, sink_t sink);
	genError_t UserDisconnect(connection_t connID);
	genError_t UserSetVolume(sink_t sink, volume_t volume);
	genError_t connect(source_t source, sink_t sink, genHandle_t* handle = NULL);
	genError_t disconnect(connection_t ID);
	genError_t setVolume(sink_t sink, volume_t volume);
	genError_t interruptRequest(source_t interruptSource, sink_t sink, genInt_t* interruptID);
	genError_t setSourceVolume (source_t source, volume_t volume);
	genError_t setSourceMute (source_t source);
	genError_t setSourceUnMute (source_t source);

	genError_t getRoute(const bool onlyfree, const source_t source, const sink_t sink, QList<genRoute_t>* ReturnList);
	connection_t returnConnectionIDforSinkSource (sink_t sink, source_t source);
	source_t returnSourceIDfromName(const QString name);
	sink_t returnSinkIDfromName(const QString name);

	QList<ConnectionType> getListConnections();
	QList<SinkType> getListSinks();
	QList<SourceType> getListSources();

	void emitSignalConnect();

	Router* returnRouter();
	DataBaseHandler* returnDatabaseHandler();
	RoutingReceiver* returnReceiver();
	DBusCommandInterface* returnCommandInterface();

	void registerDatabasehandler(DataBaseHandler * handler);
	void registerRouter(Router* router);
	void registerBushandler(Bushandler* handler);
	void registerHookEngine(HookHandler* handler);
	void registerReceiver(RoutingReceiver* receiver);
	void registerCommandInterface(DBusCommandInterface* command);

	void addQueue(Queue* queue);
	genError_t removeQueue(Queue* queue);

signals:
	void signal_connectionChanged();
	void signal_numberOfSinksChanged();
	void signal_numberOfSourcesChanged();

private:
	DataBaseHandler* m_databaseHandler; //!< pointer to the DataBasehandler Class
	Router* m_router; //!< pointer to the Router Class
	Bushandler* m_busHandler; //!< pointer to the Bushandler Class
	HookHandler* m_hookHandler; //!< pointer to the HookHandler CLass
	RoutingReceiver* m_receiver; //!< pointer to the Routing receiver Class
	DBusCommandInterface* m_command; //!< pointer to the command Interface Class

	QList<Queue*> m_queueList; //!< List of pointers to all running queues
};

#endif /* AUDIOMANAGERCORE_H_ */
