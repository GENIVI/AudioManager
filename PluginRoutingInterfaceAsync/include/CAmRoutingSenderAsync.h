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

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "routing/IAmRoutingSend.h"
#include "IAmRoutingReceiverShadow.h"
#include <semaphore.h>
#include <memory.h>
#include <map>

namespace am
{

class CAmWorkerThreadPool;

/**
 * Base class for workers implements everything that is needed to implement a workerthread
 * inherit from this class when adding new workers
 */
class CAmWorker
{
public:
    CAmWorker(CAmWorkerThreadPool* pool);
    virtual ~CAmWorker()
    {};
    /**
     * needs to be overwritten, this function is called when the worker should start to work
     */
    void virtual start2work()=0;

    /**
     * needs to be overwritten, this function is called when the worker thread is canceled. Should be used for
     * clean up and sending important messages
     */
    void virtual cancelWork()=0;
    /**
     * waits for a semaphore with a timeout. This leaves the Threadpool the chance to interrupt the processing
     * You should call whenever waiting on some event, even with time=0 in order to make sure that cancel events are
     * received
     * @param time time until timeout in timespec format
     * @return true if thread is canceled. Then just return start2work function so that the thread is given back to the pool
     */
    bool timedWait(timespec time);

    /**
     * the semaphore for cancellation is set by the thread automatically...
     * @param cancel
     */
    void setCancelSempaphore(sem_t* cancel);
    CAmWorkerThreadPool* pPool;
private:
    sem_t* mCancelSem; //<! semaphore for cancellation
};

/**
 * This class handles the threadpool
 */
class CAmWorkerThreadPool
{
public:
    /**
     * creates the pool. Give the max number of threads as argument
     * @param numThreads max number of threads
     */
    CAmWorkerThreadPool(int numThreads);
    virtual ~CAmWorkerThreadPool();

    /**
     * assigns a thread to a worker class and starts working.
     * @param worker
     * @return the actual assigned workerID or -1 in case no thread is free
     */
    int16_t startWork(CAmWorker* worker);
    /**
     * cancels a thread
     * @param workerID thread to be canceled
     * @return true if thread was found, false if not
     */
    bool cancelWork(int workerID);

    /**
     * the workers call this function upon completion of their task
     * @param threadID
     */
    void finishedWork(pthread_t threadID);

private:
    static void* CAmWorkerThread(void* data);
    int mNumThreads;
    struct threadInfo_s
    {
        uint16_t workerID;
        pthread_t threadID;
        bool busy;
        sem_t block;
        sem_t cancel;
        CAmWorker *worker;
    };
    std::vector<threadInfo_s> mListWorkers; //<! list of all workers
    static pthread_mutex_t mBlockingMutex; //<! mutex to block the acces of the list
};

class CAmRoutingSenderAsync: public IAmRoutingSend
{
public:
    CAmRoutingSenderAsync();
    virtual ~CAmRoutingSenderAsync();
    am_Error_e startupInterface(IAmRoutingReceive* routingreceiveinterface) ;
    void setRoutingReady(const uint16_t handle) ;
    void setRoutingRundown(const uint16_t handle) ;
    am_Error_e asyncAbort(const am_Handle_s handle) ;
    am_Error_e asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat) ;
    am_Error_e asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID) ;
    am_Error_e asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
    am_Error_e asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time) ;
    am_Error_e asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state) ;
    am_Error_e asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& listSoundProperties) ;
    am_Error_e asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty) ;
    am_Error_e asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& listSoundProperties) ;
    am_Error_e asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty) ;
    am_Error_e asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time) ;
    am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState) ;
    am_Error_e returnBusName(std::string& BusName) const ;
    void getInterfaceVersion(std::string& version) const ;

    /**
     * threadafe insert of route and connectionID
     * @param connectionID
     * @param route
     */
    void insertConnectionSafe(am_connectionID_t connectionID, am_RoutingElement_s route);

    /**
     * threadsafe removal of a connection
     * @param
     */
    void removeConnectionSafe(am_connectionID_t);

    /**
     * threadsafe remove of a handle
     * @param handle
     */
    void removeHandleSafe(uint16_t handle);

    /**
     * threadsafe update of Sinkvolume
     * @param sinkID
     * @param volume
     */
    void updateSinkVolumeSafe(am_sinkID_t sinkID, am_volume_t volume);

    /**
     * threadsafe update of SourceVolume
     * @param sourceID
     * @param volume
     */
    void updateSourceVolumeSafe(am_sourceID_t sourceID, am_volume_t volume);

    /**
     * threadsafe update of sourceState
     * @param sourceID
     * @param state
     */
    void updateSourceStateSafe(am_sourceID_t sourceID, am_SourceState_e state);

    /**
     * threadsafe update of sinkSoundProperty
     * @param sinkID
     * @param soundProperty
     */
    void updateSinkSoundPropertySafe(am_sinkID_t sinkID, am_SoundProperty_s soundProperty);

    /**
     * threadsafe update of sourceSoundProperty
     * @param sourceID
     * @param soundProperty
     */
    void updateSourceSoundPropertySafe(am_sourceID_t sourceID, am_SoundProperty_s soundProperty);

    /**
     * threadsafe update of domainstate
     * @param domainID
     * @param domainState
     */
    void updateDomainstateSafe(am_domainID_t domainID, am_DomainState_e domainState);

    void updateDomainListSafe(std::vector<am_Domain_s> listDomains);

    void updateSourceListSafe(std::vector<am_Source_s> listSource);

    void updateSinkListSafe(std::vector<am_Sink_s> listSinks);

private:
    /**
     * Extra thread that handles dbus stimulation for interrupt tests
     * This is a very very very basic implementation of the dbus interface
     * there is not failure handling, nothing.
     * it is used just for testing, not intended to be used otherwise...
     * @param data
     */
    std::vector<am_Domain_s> createDomainTable();
    std::vector<am_Sink_s> createSinkTable();
    std::vector<am_Source_s> createSourceTable();
    std::vector<am_Gateway_s> createGatewayTable();
    IAmRoutingReceiverShadow* mShadow;
    IAmRoutingReceive* mReceiveInterface;
    CAmSocketHandler *mSocketHandler;
    std::vector<am_Domain_s> mDomains;
    std::vector<am_Sink_s> mSinks;
    std::vector<am_Source_s> mSources;
    std::vector<am_Gateway_s> mGateways;
    std::map<uint16_t, int16_t> mMapHandleWorker;
    std::map<am_connectionID_t, am_RoutingElement_s> mMapConnectionIDRoute;
    CAmWorkerThreadPool mPool;
    pthread_t mInterruptThread;
    static pthread_mutex_t mMapConnectionMutex;
    static pthread_mutex_t mMapHandleWorkerMutex;
    static pthread_mutex_t mSinksMutex;
    static pthread_mutex_t mSourcesMutex;
    static pthread_mutex_t mDomainsMutex;
};

/**
 * worker to for connection
 */
class asycConnectWorker: public CAmWorker
{
public:
    asycConnectWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_Handle_s mHandle;
    am_connectionID_t mConnectionID;
    am_sourceID_t mSourceID;
    am_sinkID_t mSinkID;
    am_ConnectionFormat_e mConnectionFormat;
};

/**
 * worker for disconnecting
 */
class asycDisConnectWorker: public CAmWorker
{
public:
    asycDisConnectWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_Handle_s handle, const am_connectionID_t connectionID);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_Handle_s mHandle;
    am_connectionID_t mConnectionID;
    am_ConnectionFormat_e mConnectionFormat;
};

/**
 * worker to for connection
 */

#include <semaphore.h>
#include <sys/signalfd.h>
#include <signal.h>

class asyncSetSinkVolumeWorker: public CAmWorker
{
public:
    asyncSetSinkVolumeWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_volume_t currentVolume, const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_volume_t mCurrentVolume;
    am_Handle_s mHandle;
    am_sinkID_t mSinkID;
    am_volume_t mVolume;
    am_RampType_e mRamp;
    am_time_t mTime;
};

class asyncSetSourceVolumeWorker: public CAmWorker
{
public:
    asyncSetSourceVolumeWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_volume_t currentVolume, const am_Handle_s handle, const am_sourceID_t SourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_volume_t mCurrentVolume;
    am_Handle_s mHandle;
    am_sourceID_t mSourceID;
    am_volume_t mVolume;
    am_RampType_e mRamp;
    am_time_t mTime;
};

class asyncSetSourceStateWorker: public CAmWorker
{
public:
    asyncSetSourceStateWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_Handle_s mHandle;
    am_sourceID_t mSourceID;
    am_SourceState_e mSourcestate;
};

class asyncSetSinkSoundPropertyWorker: public CAmWorker
{
public:
    asyncSetSinkSoundPropertyWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_Handle_s handle, const am_SoundProperty_s soundProperty, const am_sinkID_t sinkID);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_Handle_s mHandle;
    am_sinkID_t mSinkID;
    am_SoundProperty_s mSoundProperty;
};

class asyncSetSourceSoundPropertyWorker: public CAmWorker
{
public:
    asyncSetSourceSoundPropertyWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_Handle_s handle, const am_SoundProperty_s soundProperty, const am_sourceID_t sourceID);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_Handle_s mHandle;
    am_sourceID_t mSourceID;
    am_SoundProperty_s mSoundProperty;
};

class asyncDomainStateChangeWorker: public CAmWorker
{
public:
    asyncDomainStateChangeWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const am_domainID_t domainID, const am_DomainState_e domainState);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    am_domainID_t mDomainID;
    am_DomainState_e mDomainState;
};

class syncRegisterWorker: public CAmWorker
{
public:
    syncRegisterWorker(CAmRoutingSenderAsync * asyncSender, CAmWorkerThreadPool* pool, IAmRoutingReceiverShadow* shadow, const std::vector<am_Domain_s> domains, const std::vector<am_Sink_s> sinks, const std::vector<am_Source_s> sources, const  uint16_t handle);
    void start2work();
    void cancelWork();
private:
    CAmRoutingSenderAsync * mAsyncSender;
    IAmRoutingReceiverShadow *mShadow;
    std::vector<am_Domain_s> mListDomains;
    std::vector<am_Sink_s> mListSinks;
    std::vector<am_Source_s> mListSources;
    uint16_t mHandle;
};

}

#endif /* ROUTINGSENDER_H_ */
