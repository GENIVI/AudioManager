/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger DbusPlugin
 *
 * \file RoutingSender.cpp
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

#include "RoutingSenderAsyn.h"
#include "DLTWrapper.h"
#include <algorithm>
#include <vector>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <cassert>
#include <sstream>
#include <string>
#include <dbus/dbus.h>

using namespace am;

DLT_DECLARE_CONTEXT(PluginRoutingAsync)

extern "C" RoutingSendInterface* PluginRoutingInterfaceAsyncFactory()
{
    return (new AsyncRoutingSender());
}

extern "C" void destroyRoutingPluginInterfaceAsync(RoutingSendInterface* routingSendInterface)
{
    delete routingSendInterface;
}

pthread_mutex_t AsyncRoutingSender::mMapConnectionMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t AsyncRoutingSender::mMapHandleWorkerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t AsyncRoutingSender::mSinksMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t AsyncRoutingSender::mSourcesMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t AsyncRoutingSender::mDomainsMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t WorkerThreadPool::mBlockingMutex = PTHREAD_MUTEX_INITIALIZER;

//void* AsyncRoutingSender::InterruptEvents(void *data)
//{
//    RoutingReceiverAsyncShadow *shadow=(RoutingReceiverAsyncShadow *)data;
//    DBusError err;
//    DBusMessage* msg;
//    DBusConnection* conn;
//    dbus_error_init(&err);
//    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
//    dbus_uint32_t serial = 0;
//    DBusMessage* reply;
//    DBusMessageIter args;
//    dbus_bus_request_name(conn, "org.genivi.test",DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
//
//    while (dbus_connection_read_write_dispatch(conn, -1))
//    {
//        dbus_connection_read_write(conn, 0);
//        msg = dbus_connection_pop_message(conn);
//
//        if (dbus_message_is_method_call(msg, "org.genivi.test", "timingChanged"))
//        {
//            am_connectionID_t connectionID;
//            am_timeSync_t delay;
//            dbus_message_iter_init(msg, &args);
//            dbus_message_iter_get_basic(&args,(void*) &connectionID);
//            dbus_message_iter_next(&args);
//            dbus_message_iter_get_basic(&args,(void*) &delay);
//            reply = dbus_message_new_method_return(msg);
//            dbus_message_iter_init_append(reply, &args);
//            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &connectionID);
//            dbus_connection_send(conn, reply, &serial);
//            shadow->hookTimingInformationChanged(connectionID,delay);
//            dbus_message_unref(reply);
//        }
//        else if (dbus_message_is_method_call(msg, "org.genivi.test", "SinkAvailablityStatusChange"))
//        {
//            am_sinkID_t sinkID;
//            am_Availability_s availability;
//            dbus_message_iter_init(msg, &args);
//            dbus_message_iter_get_basic(&args,(void*) &sinkID);
//            reply = dbus_message_new_method_return(msg);
//            dbus_message_iter_init_append(reply, &args);
//            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &sinkID);
//            dbus_connection_send(conn, reply, &serial);
//            shadow->hookSinkAvailablityStatusChange(sinkID,availability);
//            dbus_message_unref(reply);
//        }
//        else if (dbus_message_is_method_call(msg, "org.genivi.test", "SourceAvailablityStatusChange"))
//        {
//            am_sourceID_t sourceID;
//            am_Availability_s availability;
//            dbus_message_iter_init(msg, &args);
//            dbus_message_iter_get_basic(&args,(void*) &sourceID);
//            reply = dbus_message_new_method_return(msg);
//            dbus_message_iter_init_append(reply, &args);
//            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &sourceID);
//            dbus_connection_send(conn, reply, &serial);
//            shadow->hookSourceAvailablityStatusChange(sourceID,availability);
//            dbus_message_unref(reply);
//        }
//        else if (dbus_message_is_method_call(msg, "org.genivi.test", "InterruptStatusChange"))
//        {
//            am_sourceID_t sourceID;
//
//            am_InterruptState_e state=IS_UNKNOWN;
//            dbus_message_iter_init(msg, &args);
//            dbus_message_iter_get_basic(&args,(void*) &sourceID);
//            reply = dbus_message_new_method_return(msg);
//            dbus_message_iter_init_append(reply, &args);
//            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &sourceID);
//            dbus_connection_send(conn, reply, &serial);
//            shadow->hookInterruptStatusChange(sourceID,state);
//            dbus_message_unref(reply);
//        }
//        dbus_connection_flush(conn);
//    }
//    return NULL;
//}

        void *WorkerThreadPool::WorkerThread(void* data)
        {
            threadInfo_s *myInfo=(threadInfo_s*)data;
            while (1)
            {
                sem_wait(&myInfo->block);
                pthread_mutex_lock(&mBlockingMutex);
                Worker* actWorker=myInfo->worker;
                pthread_mutex_unlock(&mBlockingMutex);
                actWorker->setCancelSempaphore(&myInfo->cancel);
                actWorker->start2work();
                actWorker->pPool->finishedWork(myInfo->threadID);
            }
            return NULL;
        }

        WorkerThreadPool::WorkerThreadPool(int numThreads):
        mNumThreads(numThreads)
        {
            int workerID=0;
            mListWorkers.resize(mNumThreads);
            for (int i=0;i<mNumThreads;i++)
            {
                sem_init(&mListWorkers[i].block,NULL,NULL);
                sem_init(&mListWorkers[i].cancel,NULL,NULL);
                mListWorkers[i].busy=false;
                mListWorkers[i].workerID=++workerID;
                pthread_create(&mListWorkers[i].threadID,NULL,&WorkerThreadPool::WorkerThread,(void*)&mListWorkers[i]);
            }
        }

        int16_t WorkerThreadPool::startWork(Worker *worker)
        {
            pthread_mutex_lock(&mBlockingMutex);
            std::vector<threadInfo_s>::iterator it=mListWorkers.begin();
            for(;it!=mListWorkers.end();++it)
            {
                if(!it->busy)
                {
                    it->worker=worker;
                    it->busy=true;
                    pthread_mutex_unlock(&mBlockingMutex);
                    sem_post(&it->block);
                    return ((int)it->workerID);
                }
            }
            pthread_mutex_unlock(&mBlockingMutex);
            return (-1);
        }

        bool WorkerThreadPool::cancelWork(int workerID)
        {
            std::vector<threadInfo_s>::iterator it=mListWorkers.begin();
            for(;it!=mListWorkers.end();++it)
            {
                if(it->workerID==workerID && it->busy)
                {
                    sem_post(&it->cancel);
                    return (true);
                }
            }
            return (false);
        }

        void WorkerThreadPool::finishedWork(pthread_t threadID)
        {
            pthread_mutex_lock(&mBlockingMutex);
            std::vector<threadInfo_s>::iterator it=mListWorkers.begin();
            for(;it!=mListWorkers.end();++it)
            {
                if(it->threadID==threadID)
                {
                    it->busy=false;
                    delete it->worker;
                    break;
                }
            }
            pthread_mutex_unlock(&mBlockingMutex);
        }

        WorkerThreadPool::~WorkerThreadPool()
        {
            for (int i=0;i<mNumThreads;i++)
            {
                pthread_cancel(mListWorkers[i].threadID);
            }
        }

        Worker::Worker(WorkerThreadPool *pool):
        pPool(pool), mCancelSem()
        {
        }

        void Worker::setCancelSempaphore(sem_t* cancel)
        {
            mCancelSem=cancel;
        }

        bool Worker::timedWait(timespec timer)
        {
            timespec temp;
            if(clock_gettime(0, &temp)==-1)
            {
                logError("Worker::timedWait error on getting time");
            }
            temp.tv_nsec+=timer.tv_nsec;
            temp.tv_sec+=timer.tv_sec;
            //if(sem_wait(mCancelSem)==-1)
        if (sem_timedwait(mCancelSem,&temp)==-1)
        {
            //a timeout happened
            if(errno == ETIMEDOUT)
            {
                logInfo("Worker::timedWait timed out - no bug !");
                return (false);
            }
            else //failure in waiting, nevertheless, we quit the thread...
            {
                logError("Worker::timedWait semaphore waiting error");
                return (true);
            }
        }
        logError("Worker::timedWait semaphore waiting error");
        this->cancelWork();
        return (true);
    }

    AsyncRoutingSender::AsyncRoutingSender():
    mReceiveInterface(0), mDomains(createDomainTable()), mSinks(createSinkTable()), mSources ( createSourceTable ( ) ), mGateways ( createGatewayTable ( ) ), mMapHandleWorker ( ) , mMapConnectionIDRoute ( ) , mPool (10)
{
}

AsyncRoutingSender::~AsyncRoutingSender()
{
    delete mShadow;
}

am_Error_e AsyncRoutingSender::startupInterface(RoutingReceiveInterface *routingreceiveinterface)
{
    //first, create the Shadow:
    assert(routingreceiveinterface!=0);
    mReceiveInterface = routingreceiveinterface;
    SocketHandler* handler;
    routingreceiveinterface->getSocketHandler(handler);
    mShadow = new RoutingReceiverAsyncShadow(routingreceiveinterface, handler);
    return E_OK;
}

void AsyncRoutingSender::setRoutingReady(const uint16_t handle)
{
    syncRegisterWorker *worker = new syncRegisterWorker(this, &mPool, mShadow, mDomains, mSinks, mSources, handle);

    if ((mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncConnect not enough threads!");
        delete worker;
    }

    //gateways
//	std::vector<am_Gateway_s>::iterator gatewayIter=mGateways.begin();
//	for(;gatewayIter!=mGateways.end();++gatewayIter)
//	{
//		am_gatewayID_t gatewayID;
//		gatewayIter->domainSinkID=mDomains[0].domainID;
//		gatewayIter->domainSourceID=mDomains[1].domainID;
//		gatewayIter->controlDomainID=mDomains[0].domainID;
//		if((eCode=mReceiveInterface->registerGateway(*gatewayIter,gatewayID))!=E_OK)
//		{
//			logError("AsyncRoutingSender::routingInterfacesReady error on registering gateway, failed with",eCode));
//		}
//		gatewayIter->gatewayID=gatewayID;
//	}

    //create thread for interrupts, but only if we are testing - otherwise we get 100% cpu load:
    //todo: find a solution for the 100% dbus load to uncomment this and make interrupt tests work
    //pthread_create(&mInterruptThread,NULL,&AsyncRoutingSender::InterruptEvents,&mShadow);
}

void AsyncRoutingSender::setRoutingRundown(const uint16_t handle)
{
    assert(mReceiveInterface!=0);
}

am_Error_e AsyncRoutingSender::asyncAbort(const am_Handle_s handle)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);

    //first check if we know the handle
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    std::map<uint16_t, int16_t>::iterator iter = mMapHandleWorker.begin();
    if (mMapHandleWorker.find(handle.handle) == mMapHandleWorker.end())
    {
        pthread_mutex_unlock(&mMapHandleWorkerMutex);
        return (E_NON_EXISTENT);
    }
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    //ok, cancel the action:
    if (mPool.cancelWork(iter->second))
        return (E_OK);
    return (E_UNKNOWN);
}

am_Error_e AsyncRoutingSender::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_CONNECT);
    assert(connectionID!=0);
    assert(sinkID!=0);
    assert(sourceID!=0);

    //check if we can take the job
    am_Sink_s sink;
    am_Source_s source;
    int16_t work = -1;

    //find the sink
    std::vector<am_Sink_s>::iterator sinkIter = mSinks.begin();
    for (; sinkIter != mSinks.end(); ++sinkIter)
    {
        if (sinkIter->sinkID == sinkID)
        {
            sink = *sinkIter;
            break;
        }
    }
    if (sinkIter == mSinks.end())
        return (E_NON_EXISTENT); //not found!

    //find the source
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            source = *sourceIter;
            break;
        }
    }
    if (sourceIter == mSources.end())
        return (E_NON_EXISTENT); //not found!

    //check the format
    if (std::find(source.listConnectionFormats.begin(), source.listConnectionFormats.end(), connectionFormat) == source.listConnectionFormats.end())
        return (E_WRONG_FORMAT);
    if (std::find(sink.listConnectionFormats.begin(), sink.listConnectionFormats.end(), connectionFormat) == sink.listConnectionFormats.end())
        return (E_WRONG_FORMAT);

    //the operation is ok, lets create a worker, assign it to a task in the task pool
    asycConnectWorker *worker = new asycConnectWorker(this, &mPool, mShadow, handle, connectionID, sourceID, sinkID, connectionFormat);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncConnect not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_DISCONNECT);
    assert(connectionID!=0);

    //check if we can take the job
    int16_t work = -1;

    pthread_mutex_lock(&mMapConnectionMutex);
    if (mMapConnectionIDRoute.find(connectionID) == mMapConnectionIDRoute.end())
    {
        pthread_mutex_unlock(&mMapConnectionMutex);
        return (E_NON_EXISTENT);
    }
    pthread_mutex_unlock(&mMapConnectionMutex);

    //the operation is ok, lets create a worker, assign it to a task in the task pool
    asycDisConnectWorker *worker = new asycDisConnectWorker(this, &mPool, mShadow, handle, connectionID);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncDisconnect not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_SETSINKVOLUME);
    assert(sinkID!=0);

    //check if we can take the job
    am_Sink_s sink;
    int16_t work = -1;

    //find the sink
    pthread_mutex_lock(&mSinksMutex);
    std::vector<am_Sink_s>::iterator sinkIter = mSinks.begin();
    for (; sinkIter != mSinks.end(); ++sinkIter)
    {
        if (sinkIter->sinkID == sinkID)
        {
            sink = *sinkIter;
            break;
        }
    }
    pthread_mutex_unlock(&mSinksMutex);
    if (sinkIter == mSinks.end())
        return (E_NON_EXISTENT); //not found!

    asyncSetSinkVolumeWorker *worker = new asyncSetSinkVolumeWorker(this, &mPool, mShadow, sinkIter->volume, handle, sinkID, volume, ramp, time);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncSetSinkVolume not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_SETSOURCEVOLUME);
    assert(sourceID!=0);

    //check if we can take the job
    am_Source_s source;
    int16_t work = -1;

    //find the sink
    pthread_mutex_lock(&mSourcesMutex);
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            source = *sourceIter;
            break;
        }
    }
    pthread_mutex_unlock(&mSourcesMutex);
    if (sourceIter == mSources.end())
        return (E_NON_EXISTENT); //not found!

    asyncSetSourceVolumeWorker *worker = new asyncSetSourceVolumeWorker(this, &mPool, mShadow, sourceIter->volume, handle, sourceID, volume, ramp, time);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncSetSourceVolume not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_SETSOURCESTATE);
    assert(sourceID!=0);

    //check if we can take the job
    am_Source_s source;
    int16_t work = -1;

    //find the source
    pthread_mutex_lock(&mSourcesMutex);
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            source = *sourceIter;
            break;
        }
    }
    pthread_mutex_unlock(&mSourcesMutex);
    if (sourceIter == mSources.end())
        return (E_NON_EXISTENT); //not found!

    asyncSetSourceStateWorker *worker = new asyncSetSourceStateWorker(this, &mPool, mShadow, handle, sourceID, state);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncSetSourceState not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_SETSINKSOUNDPROPERTY);
    assert(sinkID!=0);

    //check if we can take the job
    am_Sink_s sink;
    int16_t work = -1;

    //find the sink
    pthread_mutex_lock(&mSinksMutex);
    std::vector<am_Sink_s>::iterator sinkIter = mSinks.begin();
    for (; sinkIter != mSinks.end(); ++sinkIter)
    {
        if (sinkIter->sinkID == sinkID)
        {
            sink = *sinkIter;
            break;
        }
    }
    pthread_mutex_unlock(&mSinksMutex);
    if (sinkIter == mSinks.end())
        return (E_NON_EXISTENT); //not found!

    asyncSetSinkSoundPropertyWorker *worker = new asyncSetSinkSoundPropertyWorker(this, &mPool, mShadow, handle, soundProperty, sinkID);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncSetSinkSoundProperty not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    //todo: implement crossfader
    (void) handle;
    (void) crossfaderID;
    (void) hotSink;
    (void) rampType;
    (void) time;
    return E_NOT_USED;
}

am_Error_e AsyncRoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    assert(mReceiveInterface!=0);
    assert(domainID!=0);

    //check if we can take the job
    am_Domain_s domain;
    int16_t work = -1;

    //find the sink
    pthread_mutex_lock(&mDomainsMutex);
    std::vector<am_Domain_s>::iterator domainIter = mDomains.begin();
    for (; domainIter != mDomains.end(); ++domainIter)
    {
        if (domainIter->domainID == domainID)
        {
            domain = *domainIter;
            break;
        }
    }
    pthread_mutex_unlock(&mDomainsMutex);
    if (domainIter == mDomains.end())
        return (E_NON_EXISTENT); //not found!

    asyncDomainStateChangeWorker *worker = new asyncDomainStateChangeWorker(this, &mPool, mShadow, domainID, domainState);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::setDomainState not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    return (E_OK);

}

am_Error_e AsyncRoutingSender::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    assert(mReceiveInterface!=0);
    assert(handle.handle!=0);
    assert(handle.handleType==H_SETSOURCESOUNDPROPERTY);
    assert(sourceID!=0);

    //check if we can take the job
    am_Source_s source;
    int16_t work = -1;

    //find the source
    pthread_mutex_lock(&mSourcesMutex);
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            source = *sourceIter;
            break;
        }
    }
    pthread_mutex_unlock(&mSourcesMutex);
    if (sourceIter == mSources.end())
        return (E_NON_EXISTENT); //not found!

    asyncSetSourceSoundPropertyWorker *worker = new asyncSetSourceSoundPropertyWorker(this, &mPool, mShadow, handle, soundProperty, sourceID);
    if ((work = mPool.startWork(worker)) == -1)
    {
        logError("AsyncRoutingSender::asyncSetSourceState not enough threads!");
        delete worker;
        return (E_NOT_POSSIBLE);
    }

    //save the handle related to the workerID
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    mMapHandleWorker.insert(std::make_pair(handle.handle, work));
    pthread_mutex_unlock(&mMapHandleWorkerMutex);

    return (E_OK);
}

am_Error_e AsyncRoutingSender::returnBusName(std::string & BusName) const
{
    BusName = "RoutingAsync";
    return (E_OK);
}

std::vector<am_Domain_s> AsyncRoutingSender::createDomainTable()
{
    //just write two domains into the table and return it
    std::vector<am_Domain_s> table;
    am_Domain_s item;
    item.busname = "RoutingAsync";
    item.domainID = 0;
    item.early = false;
    item.name = "AsyncDomain1";
    item.nodename = "AsyncNode1";
    item.state = DS_CONTROLLED;
    table.push_back(item);
    item.busname = "RoutingAsync";
    item.domainID = 0;
    item.early = false;
    item.name = "AsyncDomain2";
    item.nodename = "AsyncNode2";
    item.state = DS_CONTROLLED;
    table.push_back(item);
    return (table);
}

std::vector<am_Sink_s> AsyncRoutingSender::createSinkTable()
{
    //create a bunch full of sinks
    std::vector<am_Sink_s> table;
    am_Sink_s item;
    am_SoundProperty_s sp;
    sp.type = SP_EXAMPLE_BASS;
    sp.value = 0;

    std::vector<am_MainSoundProperty_s> listMainSoundProperties;
    am_MainSoundProperty_s msp;
    msp.type = MSP_EXAMPLE_BASS;
    msp.value = 5;
    listMainSoundProperties.push_back(msp);
    msp.type = MSP_EXAMPLE_MID;
    listMainSoundProperties.push_back(msp);
    msp.type = MSP_EXAMPLE_TREBLE;
    listMainSoundProperties.push_back(msp);
    for (int16_t i = 0; i <= 10; i++)
    {
        std::stringstream temp;
        temp << i;
        item.domainID = 0; //we cannot know this when the table is created !
        item.name = "mySink" + temp.str();
        item.sinkID = i; //take fixed ids to make thins easy
        item.sinkClassID = 1;
        item.volume = 0;
        item.available.availability = A_AVAILABLE;
        item.available.availabilityReason = AR_UNKNOWN;
        item.listSoundProperties.push_back(sp);
        item.listMainSoundProperties = listMainSoundProperties;
        item.visible = true;
        item.listConnectionFormats.push_back(CF_GENIVI_ANALOG);
        item.muteState = MS_MUTED;
        item.mainVolume = 0;
        table.push_back(item);
    }
    return (table);
}

std::vector<am_Source_s> AsyncRoutingSender::createSourceTable()
{
    //create a bunch full of sources
    std::vector<am_Source_s> table;
    am_Source_s item;
    for (int16_t i = 0; i <= 10; i++)
    {
        std::stringstream temp;
        temp << i;
        item.domainID = 0; //we cannot know this when the table is created !
        item.name = "mySource" + temp.str();
        item.sourceState = SS_OFF;
        item.sourceID = i; //take fixed ids to make thins easy
        item.sourceClassID = 1;
        item.volume = 0;
        item.visible = true;
        item.available.availability = A_AVAILABLE;
        item.available.availabilityReason = AR_UNKNOWN;
        item.listConnectionFormats.push_back(CF_GENIVI_ANALOG);
        table.push_back(item);
    }
    return (table);
}

void AsyncRoutingSender::insertConnectionSafe(am_connectionID_t connectionID, am_RoutingElement_s route)
{
    pthread_mutex_lock(&mMapConnectionMutex);
    mMapConnectionIDRoute.insert(std::make_pair(connectionID, route));
    pthread_mutex_unlock(&mMapConnectionMutex);
}

void AsyncRoutingSender::removeHandleSafe(uint16_t handle)
{
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    if (!mMapHandleWorker.erase(handle))
    {
        logError("AsyncRoutingSender::removeHandle could not remove handle");
    }
    pthread_mutex_unlock(&mMapHandleWorkerMutex);
}

void AsyncRoutingSender::removeConnectionSafe(am_connectionID_t connectionID)
{
    pthread_mutex_lock(&mMapConnectionMutex);
    if (!mMapConnectionIDRoute.erase(connectionID))
    {
        logError("AsyncRoutingSender::removeConnectionSafe could not remove connection");
    }
    pthread_mutex_unlock(&mMapConnectionMutex);
}

void AsyncRoutingSender::updateSinkVolumeSafe(am_sinkID_t sinkID, am_volume_t volume)
{
    pthread_mutex_lock(&mSinksMutex);
    std::vector<am_Sink_s>::iterator sinkIter = mSinks.begin();
    for (; sinkIter != mSinks.end(); ++sinkIter)
    {
        if (sinkIter->sinkID == sinkID)
        {
            sinkIter->volume = volume;
            break;
        }
    }
    pthread_mutex_unlock(&mSinksMutex);
}

void am::AsyncRoutingSender::updateSourceVolumeSafe(am_sourceID_t sourceID, am_volume_t volume)
{
    pthread_mutex_lock(&mSourcesMutex);
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            sourceIter->volume = volume;
            break;
        }
    }
    pthread_mutex_unlock(&mSourcesMutex);
}

void am::AsyncRoutingSender::updateSourceStateSafe(am_sourceID_t sourceID, am_SourceState_e state)
{
    pthread_mutex_lock(&mSourcesMutex);
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            sourceIter->sourceState = state;
            break;
        }
    }
    pthread_mutex_unlock(&mSourcesMutex);
}

void am::AsyncRoutingSender::updateSinkSoundPropertySafe(am_sinkID_t sinkID, am_SoundProperty_s soundProperty)
{
    pthread_mutex_lock(&mSinksMutex);
    std::vector<am_Sink_s>::iterator sinkIter = mSinks.begin();
    for (; sinkIter != mSinks.end(); ++sinkIter)
    {
        if (sinkIter->sinkID == sinkID)
        {
            std::vector<am_SoundProperty_s>::iterator spIterator = sinkIter->listSoundProperties.begin();
            for (; spIterator != sinkIter->listSoundProperties.end(); ++spIterator)
            {
                if (spIterator->type == soundProperty.type)
                {
                    spIterator->value = soundProperty.value;
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&mSinksMutex);
}

void am::AsyncRoutingSender::updateSourceSoundPropertySafe(am_sourceID_t sourceID, am_SoundProperty_s soundProperty)
{
    pthread_mutex_lock(&mSourcesMutex);
    std::vector<am_Source_s>::iterator sourceIter = mSources.begin();
    for (; sourceIter != mSources.end(); ++sourceIter)
    {
        if (sourceIter->sourceID == sourceID)
        {
            std::vector<am_SoundProperty_s>::iterator spIterator = sourceIter->listSoundProperties.begin();
            for (; spIterator != sourceIter->listSoundProperties.end(); ++spIterator)
            {
                if (spIterator->type == soundProperty.type)
                {
                    spIterator->value = soundProperty.value;
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&mSourcesMutex);
}

void am::AsyncRoutingSender::updateDomainstateSafe(am_domainID_t domainID, am_DomainState_e domainState)
{
    pthread_mutex_lock(&mDomainsMutex);
    std::vector<am_Domain_s>::iterator domainIter = mDomains.begin();
    for (; domainIter != mDomains.end(); ++domainIter)
    {
        if (domainIter->domainID == domainID)
        {
            domainIter->state = domainState;
            break;
        }
    }
    pthread_mutex_unlock(&mDomainsMutex);
}

void am::AsyncRoutingSender::updateDomainListSafe(std::vector<am_Domain_s> listDomains)
{
    pthread_mutex_lock(&mDomainsMutex);
    mDomains = listDomains;
    pthread_mutex_unlock(&mDomainsMutex);
}

void am::AsyncRoutingSender::updateSourceListSafe(std::vector<am_Source_s> listSource)
{
    pthread_mutex_lock(&mSourcesMutex);
    mSources = listSource;
    pthread_mutex_unlock(&mSourcesMutex);
}

void am::AsyncRoutingSender::updateSinkListSafe(std::vector<am_Sink_s> listSinks)
{
    pthread_mutex_lock(&mSinksMutex);
    mSinks = listSinks;
    pthread_mutex_unlock(&mSinksMutex);
}

void AsyncRoutingSender::getInterfaceVersion(std::string & version) const
{
    version = RoutingSendVersion;
}

am_Error_e AsyncRoutingSender::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    //todo: implement
    (void) handle;
    (void) sourceID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

am_Error_e AsyncRoutingSender::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    //todo: implement
    (void) handle;
    (void) sinkID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

std::vector<am_Gateway_s> AsyncRoutingSender::createGatewayTable()
{
    std::vector<am_Gateway_s> table;
    am_Gateway_s item;
    item.name = "myGateway";
    item.sinkID = 2;
    item.sourceID = 2;
    table.push_back(item);
    return (table);
}

asycConnectWorker::asycConnectWorker(AsyncRoutingSender * asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow* shadow, const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mHandle(handle), //
        mConnectionID(connectionID), //
        mSourceID(sourceID), //
        mSinkID(sinkID), //
        mConnectionFormat(connectionFormat)
{
}

void asycConnectWorker::start2work()
{
    logInfo("Start connecting");
    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 1;

    //do something for one second
    if (timedWait(t))
        return;
    am_RoutingElement_s route;
    route.sinkID = mSinkID;
    route.sourceID = mSourceID;
    route.connectionFormat = mConnectionFormat;

    //enter new connectionID into the list
    mAsyncSender->insertConnectionSafe(mConnectionID, route);

    //send the ack
    mShadow->ackConnect(mHandle, mConnectionID, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

void asycConnectWorker::cancelWork()
{
    mAsyncSender->removeHandleSafe(mHandle.handle);
    mShadow->ackConnect(mHandle, mConnectionID, E_ABORTED);
}

asycDisConnectWorker::asycDisConnectWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_Handle_s handle, const am_connectionID_t connectionID) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mHandle(handle), //
        mConnectionID(connectionID)
{
}

void asycDisConnectWorker::start2work()
{
    logInfo("Start disconnecting");
    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 1;

    //do something for one second
    if (timedWait(t))
        return;
    am_RoutingElement_s route;

    //enter new connectionID into the list
    mAsyncSender->insertConnectionSafe(mConnectionID, route);

    //send the ack
    mShadow->ackDisconnect(mHandle, mConnectionID, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);

}

void asycDisConnectWorker::cancelWork()
{
    mAsyncSender->removeHandleSafe(mHandle.handle);
    mShadow->ackDisconnect(mHandle, mConnectionID, E_ABORTED);
}

asyncSetSinkVolumeWorker::asyncSetSinkVolumeWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_volume_t currentVolume, const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mCurrentVolume(currentVolume), //
        mHandle(handle), //
        mSinkID(sinkID), //
        mVolume(volume), //
        mRamp(ramp), //
        mTime(time)
{
}

void asyncSetSinkVolumeWorker::start2work()
{
    //todo: this implementation does not respect time and ramp....
    logInfo("Start setting sink volume");
    timespec t;
    t.tv_nsec = 10000000;
    t.tv_sec = 0;

    while (mCurrentVolume != mVolume)
    {
        if (mCurrentVolume < mVolume)
            mCurrentVolume++;
        else
            mCurrentVolume--;
        mShadow->ackSinkVolumeTick(mHandle, mSinkID, mCurrentVolume);
        if (timedWait(t))
            return;
    }

    //enter new connectionID into the list
    mAsyncSender->updateSinkVolumeSafe(mSinkID, mCurrentVolume);

    //send the ack
    mShadow->ackSetSinkVolumeChange(mHandle, mCurrentVolume, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

void asyncSetSinkVolumeWorker::cancelWork()
{
    mAsyncSender->updateSinkVolumeSafe(mSinkID, mCurrentVolume);
    mAsyncSender->removeHandleSafe(mHandle.handle);
    mShadow->ackSetSinkVolumeChange(mHandle, mCurrentVolume, E_ABORTED);
}

asyncSetSourceVolumeWorker::asyncSetSourceVolumeWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_volume_t currentVolume, const am_Handle_s handle, const am_sourceID_t SourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mCurrentVolume(currentVolume), //
        mHandle(handle), //
        mSourceID(SourceID), //
        mVolume(volume), //
        mRamp(ramp), //
        mTime(time)
{
}

void asyncSetSourceVolumeWorker::start2work()
{
    //todo: this implementation does not respect time and ramp....
    logInfo("Start setting source volume");
    timespec t;
    t.tv_nsec = 10000000;
    t.tv_sec = 0;

    while (mCurrentVolume != mVolume)
    {
        if (mCurrentVolume < mVolume)
            mCurrentVolume++;
        else
            mCurrentVolume--;
        mShadow->ackSourceVolumeTick(mHandle, mSourceID, mCurrentVolume);
        if (timedWait(t))
            return;
    }

    //enter new connectionID into the list
    mAsyncSender->updateSourceVolumeSafe(mSourceID, mCurrentVolume);

    //send the ack
    mShadow->ackSetSourceVolumeChange(mHandle, mCurrentVolume, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

void asyncSetSourceVolumeWorker::cancelWork()
{
    mAsyncSender->updateSourceVolumeSafe(mSourceID, mCurrentVolume);
    mAsyncSender->removeHandleSafe(mHandle.handle);
    mShadow->ackSetSourceVolumeChange(mHandle, mCurrentVolume, E_ABORTED);
}

asyncSetSourceStateWorker::asyncSetSourceStateWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mHandle(handle), //
        mSourceID(sourceID), //
        mSourcestate(state)
{
}

void asyncSetSourceStateWorker::start2work()
{
    logInfo("Start setting source state");
    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 1;

    //do something for one second
    if (timedWait(t))
        return;

    //enter new connectionID into the list
    mAsyncSender->updateSourceStateSafe(mSourceID, mSourcestate);

    //send the ack
    mShadow->ackSetSourceState(mHandle, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

void asyncSetSourceStateWorker::cancelWork()
{
    //send the ack
    mShadow->ackSetSourceState(mHandle, E_ABORTED);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

asyncSetSinkSoundPropertyWorker::asyncSetSinkSoundPropertyWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_Handle_s handle, const am_SoundProperty_s soundProperty, const am_sinkID_t sinkID) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mHandle(handle), //
        mSinkID(sinkID), //
        mSoundProperty(soundProperty)
{
}

void asyncSetSinkSoundPropertyWorker::start2work()
{
    logInfo("Start setting sink sound property");
    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 1;

    //do something for one second
    if (timedWait(t))
        return;

    //enter new connectionID into the list
    mAsyncSender->updateSinkSoundPropertySafe(mSinkID, mSoundProperty);

    //send the ack
    mShadow->ackSetSinkSoundProperty(mHandle, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

void asyncSetSinkSoundPropertyWorker::cancelWork()
{
    //send the ack
    mShadow->ackSetSinkSoundProperty(mHandle, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

asyncSetSourceSoundPropertyWorker::asyncSetSourceSoundPropertyWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_Handle_s handle, const am_SoundProperty_s soundProperty, const am_sourceID_t sourceID) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mHandle(handle), //
        mSourceID(sourceID), //
        mSoundProperty(soundProperty)
{
}

void asyncSetSourceSoundPropertyWorker::start2work()
{
    logInfo("Start setting source sound property");
    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 1;

    //do something for one second
    if (timedWait(t))
        return;

    //enter new connectionID into the list
    mAsyncSender->updateSourceSoundPropertySafe(mSourceID, mSoundProperty);

    //send the ack
    mShadow->ackSetSourceSoundProperty(mHandle, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

void asyncSetSourceSoundPropertyWorker::cancelWork()
{
    //send the ack
    mShadow->ackSetSourceSoundProperty(mHandle, E_OK);

    //destroy the handle
    mAsyncSender->removeHandleSafe(mHandle.handle);
}

asyncDomainStateChangeWorker::asyncDomainStateChangeWorker(AsyncRoutingSender *asyncSender, WorkerThreadPool *pool, RoutingReceiverAsyncShadow *shadow, const am_domainID_t domainID, const am_DomainState_e domainState) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mDomainID(domainID), //
        mDomainState(domainState)
{
}

void asyncDomainStateChangeWorker::start2work()
{
    //todo: sendchanged data must be in here !
    logInfo("Start setting source sound property");
    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 1;

    //do something for one second
    if (timedWait(t))
        return;

    //enter new connectionID into the list
    mAsyncSender->updateDomainstateSafe(mDomainID, mDomainState);
    mShadow->hookDomainStateChange(mDomainID, mDomainState);
    //send the new status

}

void am::asyncDomainStateChangeWorker::cancelWork()
{
    //send the new status
    mShadow->hookDomainStateChange(mDomainID, mDomainState);
}

syncRegisterWorker::syncRegisterWorker(AsyncRoutingSender * asyncSender, WorkerThreadPool* pool, RoutingReceiverAsyncShadow* shadow, const std::vector<am_Domain_s> domains, const std::vector<am_Sink_s> sinks, const std::vector<am_Source_s> sources, const  uint16_t handle) :
        Worker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mListDomains(domains), //
        mListSinks(sinks), //
        mListSources(sources),
        mHandle(handle)
{
}

void syncRegisterWorker::start2work()
{
    //todo: sendchanged data must be in here !
    logInfo("Start to register stuff");

    am_Error_e eCode;

    std::vector<am_Domain_s>::iterator domainIter = mListDomains.begin();
    for (; domainIter != mListDomains.end(); ++domainIter)
    {
        am_domainID_t domainID;
        if ((eCode = mShadow->registerDomain(*domainIter, domainID)) != E_OK)
        {
            logError("syncRegisterWorker::start2work error on registering domain, failed with", eCode);
        }
        domainIter->domainID = domainID;
    }

    mAsyncSender->updateDomainListSafe(mListDomains);

    //then sources
    std::vector<am_Source_s>::iterator sourceIter = mListSources.begin();
    for (; sourceIter != mListSources.end(); ++sourceIter)
    {
        am_sourceID_t sourceID;
        //set the correct domainID
        sourceIter->domainID = mListDomains[0].domainID;
        if ((eCode = mShadow->registerSource(*sourceIter, sourceID)) != E_OK)
        {
            logError("syncRegisterWorker::start2work error on registering source, failed with", eCode);
        }
    }

    mAsyncSender->updateSourceListSafe(mListSources);

    //sinks
    std::vector<am_Sink_s>::iterator sinkIter = mListSinks.begin();
    for (; sinkIter != mListSinks.end(); ++sinkIter)
    {
        am_sinkID_t sinkID;
        //set the correct domainID
        sinkIter->domainID = mListDomains[0].domainID;
        if ((eCode = mShadow->registerSink(*sinkIter, sinkID)) != E_OK)
        {
            logError("syncRegisterWorker::start2work error on registering sink, failed with", eCode);
        }
    }

    mAsyncSender->updateSinkListSafe(mListSinks);
    mShadow->confirmRoutingReady(mHandle);
}

void syncRegisterWorker::cancelWork()
{
}

