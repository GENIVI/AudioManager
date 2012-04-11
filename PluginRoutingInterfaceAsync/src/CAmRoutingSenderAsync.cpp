/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "CAmRoutingSenderAsync.h"
#include <algorithm>
#include <vector>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <cassert>
#include <sstream>
#include <string>
#include <dbus/dbus.h>
#include "shared/CAmDltWrapper.h"

#define MAX_NS 1000000000L

using namespace am;

DLT_DECLARE_CONTEXT(PluginRoutingAsync)

extern "C" IAmRoutingSend* PluginRoutingInterfaceAsyncFactory()
{
    return (new CAmRoutingSenderAsync());
}

extern "C" void destroyRoutingPluginInterfaceAsync(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

pthread_mutex_t CAmRoutingSenderAsync::mMapConnectionMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CAmRoutingSenderAsync::mMapHandleWorkerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CAmRoutingSenderAsync::mSinksMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CAmRoutingSenderAsync::mSourcesMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CAmRoutingSenderAsync::mDomainsMutex= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CAmWorkerThreadPool::mBlockingMutex = PTHREAD_MUTEX_INITIALIZER;

void *CAmWorkerThreadPool::CAmWorkerThread(void* data)
{
    threadInfo_s *myInfo=(threadInfo_s*)data;
    while (1)
    {
        sem_wait(&myInfo->block);
        pthread_mutex_lock(&mBlockingMutex);
        CAmWorker* actWorker=myInfo->worker;
        pthread_mutex_unlock(&mBlockingMutex);
        actWorker->setCancelSempaphore(&myInfo->cancel);
        actWorker->start2work();
        actWorker->pPool->finishedWork(myInfo->threadID);
    }
    return NULL;
}

CAmWorkerThreadPool::CAmWorkerThreadPool(int numThreads):
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
        pthread_create(&mListWorkers[i].threadID,NULL,&CAmWorkerThreadPool::CAmWorkerThread,(void*)&mListWorkers[i]);
    }
}

int16_t CAmWorkerThreadPool::startWork(CAmWorker *worker)
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

bool CAmWorkerThreadPool::cancelWork(int workerID)
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

void CAmWorkerThreadPool::finishedWork(pthread_t threadID)
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

CAmWorkerThreadPool::~CAmWorkerThreadPool()
{
    for (int i=0;i<mNumThreads;i++)
    {
        pthread_cancel(mListWorkers[i].threadID);
    }
}

CAmWorker::CAmWorker(CAmWorkerThreadPool *pool):
pPool(pool), mCancelSem ( ) { }
void CAmWorker ::setCancelSempaphore ( sem_t * cancel )
{
    mCancelSem=cancel;
}

bool CAmWorker::timedWait(timespec timer)
{
    timespec temp;
    if (clock_gettime(0, &temp) == -1)
    {
        logError("Worker::timedWait error on getting time");
    }
    temp.tv_nsec += timer.tv_nsec;
    temp.tv_sec += timer.tv_sec;

    if (temp.tv_nsec >= MAX_NS)
    {
        temp.tv_sec++;
        temp.tv_nsec = temp.tv_nsec - MAX_NS;
    }
    //if(sem_wait(mCancelSem)==-1)
    if (sem_timedwait(mCancelSem, &temp) == -1)
    {
        //a timeout happened
        if (errno == ETIMEDOUT)
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

CAmRoutingSenderAsync::CAmRoutingSenderAsync() :
        mReceiveInterface(0), mDomains(createDomainTable()), mSinks(createSinkTable()), mSources(createSourceTable()), mGateways(createGatewayTable()), mMapHandleWorker(), mMapConnectionIDRoute(), mPool(10)
{
}

CAmRoutingSenderAsync::~CAmRoutingSenderAsync()
{
    delete mShadow;
}

am_Error_e CAmRoutingSenderAsync::startupInterface(IAmRoutingReceive *routingreceiveinterface)
{
    //first, create the Shadow:
    assert(routingreceiveinterface!=0);
    mReceiveInterface = routingreceiveinterface;
    CAmSocketHandler* handler;
    routingreceiveinterface->getSocketHandler(handler);
    mShadow = new IAmRoutingReceiverShadow(routingreceiveinterface, handler);
    return E_OK;
}

void CAmRoutingSenderAsync::setRoutingReady(const uint16_t handle)
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

void CAmRoutingSenderAsync::setRoutingRundown(const uint16_t handle)
{
    assert(mReceiveInterface!=0);
    mShadow->confirmRoutingRundown(handle);
}

am_Error_e CAmRoutingSenderAsync::asyncAbort(const am_Handle_s handle)
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

am_Error_e CAmRoutingSenderAsync::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
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

am_Error_e CAmRoutingSenderAsync::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
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

am_Error_e CAmRoutingSenderAsync::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
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

am_Error_e CAmRoutingSenderAsync::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
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

am_Error_e CAmRoutingSenderAsync::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
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

am_Error_e CAmRoutingSenderAsync::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
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

am_Error_e CAmRoutingSenderAsync::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    //todo: implement crossfader
    (void) handle;
    (void) crossfaderID;
    (void) hotSink;
    (void) rampType;
    (void) time;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderAsync::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
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

am_Error_e CAmRoutingSenderAsync::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
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

am_Error_e CAmRoutingSenderAsync::returnBusName(std::string & BusName) const
{
    BusName = "RoutingAsync";
    return (E_OK);
}

std::vector<am_Domain_s> CAmRoutingSenderAsync::createDomainTable()
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

std::vector<am_Sink_s> CAmRoutingSenderAsync::createSinkTable()
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

std::vector<am_Source_s> CAmRoutingSenderAsync::createSourceTable()
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

void CAmRoutingSenderAsync::insertConnectionSafe(am_connectionID_t connectionID, am_RoutingElement_s route)
{
    pthread_mutex_lock(&mMapConnectionMutex);
    mMapConnectionIDRoute.insert(std::make_pair(connectionID, route));
    pthread_mutex_unlock(&mMapConnectionMutex);
}

void CAmRoutingSenderAsync::removeHandleSafe(uint16_t handle)
{
    pthread_mutex_lock(&mMapHandleWorkerMutex);
    if (!mMapHandleWorker.erase(handle))
    {
        logError("AsyncRoutingSender::removeHandle could not remove handle");
    }
    pthread_mutex_unlock(&mMapHandleWorkerMutex);
}

void CAmRoutingSenderAsync::removeConnectionSafe(am_connectionID_t connectionID)
{
    pthread_mutex_lock(&mMapConnectionMutex);
    if (!mMapConnectionIDRoute.erase(connectionID))
    {
        logError("AsyncRoutingSender::removeConnectionSafe could not remove connection");
    }
    pthread_mutex_unlock(&mMapConnectionMutex);
}

void CAmRoutingSenderAsync::updateSinkVolumeSafe(am_sinkID_t sinkID, am_volume_t volume)
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

void am::CAmRoutingSenderAsync::updateSourceVolumeSafe(am_sourceID_t sourceID, am_volume_t volume)
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

void am::CAmRoutingSenderAsync::updateSourceStateSafe(am_sourceID_t sourceID, am_SourceState_e state)
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

void am::CAmRoutingSenderAsync::updateSinkSoundPropertySafe(am_sinkID_t sinkID, am_SoundProperty_s soundProperty)
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

void am::CAmRoutingSenderAsync::updateSourceSoundPropertySafe(am_sourceID_t sourceID, am_SoundProperty_s soundProperty)
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

void am::CAmRoutingSenderAsync::updateDomainstateSafe(am_domainID_t domainID, am_DomainState_e domainState)
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

void am::CAmRoutingSenderAsync::updateDomainListSafe(std::vector<am_Domain_s> listDomains)
{
    pthread_mutex_lock(&mDomainsMutex);
    mDomains = listDomains;
    pthread_mutex_unlock(&mDomainsMutex);
}

void am::CAmRoutingSenderAsync::updateSourceListSafe(std::vector<am_Source_s> listSource)
{
    pthread_mutex_lock(&mSourcesMutex);
    mSources = listSource;
    pthread_mutex_unlock(&mSourcesMutex);
}

void am::CAmRoutingSenderAsync::updateSinkListSafe(std::vector<am_Sink_s> listSinks)
{
    pthread_mutex_lock(&mSinksMutex);
    mSinks = listSinks;
    pthread_mutex_unlock(&mSinksMutex);
}

void CAmRoutingSenderAsync::getInterfaceVersion(std::string & version) const
{
    version = RoutingSendVersion;
}

am_Error_e CAmRoutingSenderAsync::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    //todo: implement
    (void) handle;
    (void) sourceID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderAsync::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    //todo: implement
    (void) handle;
    (void) sinkID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

std::vector<am_Gateway_s> CAmRoutingSenderAsync::createGatewayTable()
{
    std::vector<am_Gateway_s> table;
    am_Gateway_s item;
    item.name = "myGateway";
    item.sinkID = 2;
    item.sourceID = 2;
    table.push_back(item);
    return (table);
}

asycConnectWorker::asycConnectWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow* shadow, const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) :
        CAmWorker(pool), //
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

asycDisConnectWorker::asycDisConnectWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_Handle_s handle, const am_connectionID_t connectionID) :
        CAmWorker(pool), //
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

asyncSetSinkVolumeWorker::asyncSetSinkVolumeWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_volume_t currentVolume, const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) :
        CAmWorker(pool), //
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

asyncSetSourceVolumeWorker::asyncSetSourceVolumeWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_volume_t currentVolume, const am_Handle_s handle, const am_sourceID_t SourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) :
        CAmWorker(pool), //
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

asyncSetSourceStateWorker::asyncSetSourceStateWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state) :
        CAmWorker(pool), //
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

asyncSetSinkSoundPropertyWorker::asyncSetSinkSoundPropertyWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_Handle_s handle, const am_SoundProperty_s soundProperty, const am_sinkID_t sinkID) :
        CAmWorker(pool), //
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

asyncSetSourceSoundPropertyWorker::asyncSetSourceSoundPropertyWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_Handle_s handle, const am_SoundProperty_s soundProperty, const am_sourceID_t sourceID) :
        CAmWorker(pool), //
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

asyncDomainStateChangeWorker::asyncDomainStateChangeWorker(CAmRoutingSenderAsync *asyncSender, CAmWorkerThreadPool *pool, IAmRoutingReceiverShadow *shadow, const am_domainID_t domainID, const am_DomainState_e domainState) :
        CAmWorker(pool), //
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

syncRegisterWorker::syncRegisterWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const std::vector<am_Domain_s> domains, const std::vector<am_Sink_s> sinks, const std::vector<am_Source_s> sources, const uint16_t handle) :
        CAmWorker(pool), //
        mAsyncSender(asyncSender), //
        mShadow(shadow), //
        mListDomains(domains), //
        mListSinks(sinks), //
        mListSources(sources), mHandle(handle)
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

