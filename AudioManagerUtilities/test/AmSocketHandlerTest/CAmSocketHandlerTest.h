/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef SOCKETHANDLERTEST_H_
#define SOCKETHANDLERTEST_H_

#define WITH_DLT

#include <ctime>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <queue>
#include "CAmSocketHandler.h"

#undef ENABLED_SOCKETHANDLER_TEST_OUTPUT
#undef ENABLED_TIMERS_TEST_OUTPUT
#define TIMERS_CB_TOLERANCE 8.f

namespace am
{
    class IAmTimerCb
    {
    public:
        virtual ~IAmTimerCb()
        {
        }
        virtual void timerCallback(sh_timerHandle_t handle, void * userData)=0;
    };

    class IAmSignalHandler
    {
    public:
        virtual ~IAmSignalHandler()
        {
        }
        virtual void signalHandlerAction(const sh_pollHandle_t handle, const unsigned sig, void* userData)=0;
        virtual void signalHandler(int sig, siginfo_t *siginfo, void *context)=0;
    };

    class IAmSocketHandlerCb
    {
    public:
        virtual ~IAmSocketHandlerCb()
        {
        }
        virtual void receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)=0;
        virtual bool dispatchData(const sh_pollHandle_t handle, void* userData)=0;
        virtual bool check(const sh_pollHandle_t handle, void* userData)=0;
    };

    class MockIAmTimerCb: public IAmTimerCb
    {
    public:
        MOCK_CONST_METHOD2(timerCallback,
                void(sh_timerHandle_t handle, void *userData));
    };

    class MockIAmSignalHandler: public IAmSignalHandler
    {
    public:
        MOCK_METHOD3(signalHandlerAction, void (const sh_pollHandle_t handle, const unsigned sig, void* userData));
        MOCK_METHOD3(signalHandler, void(int sig, siginfo_t *siginfo, void *context));
    };

    class MockSocketHandlerCb: public IAmSocketHandlerCb
    {
    public:
        MOCK_CONST_METHOD3(receiveData,
                void(const pollfd pollfd, const sh_pollHandle_t handle, void* userData));
        MOCK_CONST_METHOD2(dispatchData,
                void(const sh_pollHandle_t handle, void* userData));
        MOCK_CONST_METHOD2(check,
                void(const sh_pollHandle_t handle, void* userData));
    };

    class CAmSamplePlugin: public MockSocketHandlerCb
    {
    public:
        enum sockType_e
        {
            UNIX, INET
        };
        CAmSamplePlugin(CAmSocketHandler *mySocketHandler, sockType_e socketType);
        virtual ~CAmSamplePlugin()
        {
        }
        ;
        void connectSocket(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
        virtual void receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
        virtual bool dispatchData(const sh_pollHandle_t handle, void* userData);
        virtual bool check(const sh_pollHandle_t handle, void* userData);
        TAmShPollFired<CAmSamplePlugin> connectFiredCB;
        TAmShPollFired<CAmSamplePlugin> receiveFiredCB;
        TAmShPollDispatch<CAmSamplePlugin> sampleDispatchCB;
        TAmShPollCheck<CAmSamplePlugin> sampleCheckCB;

    protected:
        CAmSocketHandler *mSocketHandler;
        sh_pollHandle_t mConnecthandle, mReceiveHandle;
        std::queue<std::string> msgList;
    };

    class CAmTimerSockethandlerController: public MockIAmTimerCb
    {
        CAmSocketHandler *mpSocketHandler;
        timespec mUpdateTimeout;
    public:
        explicit CAmTimerSockethandlerController(CAmSocketHandler *SocketHandler, const timespec &timeout);
        virtual ~CAmTimerSockethandlerController();

        void timerCallback(sh_timerHandle_t handle, void * userData);

        TAmShTimerCallBack<CAmTimerSockethandlerController> pTimerCallback;
    };

    class CAmTimerSignalHandler: public MockIAmTimerCb
    {
        unsigned mIndex;
        std::set<unsigned> mSignals;
        CAmSocketHandler *mpSocketHandler;
        timespec mUpdateTimeout;
    public:
        explicit CAmTimerSignalHandler(CAmSocketHandler *SocketHandler, const timespec &timeout, const std::set<unsigned> & signals);
        virtual ~CAmTimerSignalHandler();

        void timerCallback(sh_timerHandle_t handle, void * userData);

        TAmShTimerCallBack<CAmTimerSignalHandler> pTimerCallback;
    };

    class CAmTimer: public MockIAmTimerCb
    {
        CAmSocketHandler *mpSocketHandler;
        timespec mUpdateTimeout;
        int32_t mRepeats;
    public:
        explicit CAmTimer(CAmSocketHandler *SocketHandler, const timespec &timeout, const int32_t repeats = 0u);
        virtual ~CAmTimer();

        void timerCallback(sh_timerHandle_t handle, void * userData);

        TAmShTimerCallBack<CAmTimer> pTimerCallback;
    };

    class CAmTimerStressTest: public MockIAmTimerCb
    {
        CAmSocketHandler *mpSocketHandler;
        timespec mUpdateTimeout;
        int32_t mRepeats;
        int32_t mId;
        int32_t mHandle;
    public:
        explicit CAmTimerStressTest(CAmSocketHandler *SocketHandler, const timespec &timeout, const int32_t repeats = 0u);
        virtual ~CAmTimerStressTest();
        
        int32_t getId() { return mId; }
        void setId(const int32_t id) { mId=id; }

        int32_t getHandle() { return mHandle; }
        void setHandle(const int32_t id) { mHandle=id; }
        
        timespec getUpdateTimeout( ) { return mUpdateTimeout; }
        
        void timerCallback(sh_timerHandle_t handle, void * userData);

        TAmShTimerCallBack<CAmTimerStressTest> pTimerCallback;
    };
    
    class CAmTimerStressTest2: public MockIAmTimerCb
    {
        CAmSocketHandler *mpSocketHandler;
        timespec mUpdateTimeout;
        int32_t mRepeats;
        int32_t mId;
        int32_t mHandle;
    public:
        explicit CAmTimerStressTest2(CAmSocketHandler *SocketHandler, const timespec &timeout, const int32_t repeats = 0u);
        virtual ~CAmTimerStressTest2();
        
        int32_t getId() { return mId; }
        void setId(const int32_t id) { mId=id; }

        int32_t getHandle() { return mHandle; }
        void setHandle(const int32_t id) { mHandle=id; }
        
        timespec getUpdateTimeout( ) { return mUpdateTimeout; }

        void timerCallback(sh_timerHandle_t handle, void * userData);

        TAmShTimerCallBack<CAmTimerStressTest2> pTimerCallback;
    };
    
    class CAmTimerMeasurment: public MockIAmTimerCb
    {
        CAmSocketHandler *mSocketHandler;
        timespec mUpdateTimeout;
        std::chrono::time_point<std::chrono::high_resolution_clock> mUpdateTimePoint;
        std::chrono::time_point<std::chrono::high_resolution_clock> mLastInvocationTime;
        std::chrono::duration<long, std::ratio<1l, 1000000000l>> mExpected;
        int32_t mRepeats;
        void * mpUserData;
        std::string mDebugText;
    public:
        explicit CAmTimerMeasurment(CAmSocketHandler *SocketHandler, const timespec &timeout, const std::string & label, const int32_t repeats = 0u, void * userData = NULL);
        virtual ~CAmTimerMeasurment();

        void timerCallback(sh_timerHandle_t handle, void * userData);
        TAmShTimerCallBack<CAmTimerMeasurment> pTimerCallback;
    };

    class CAmSocketHandlerTest: public ::testing::Test
    {
    public:
        CAmSocketHandlerTest();
        ~CAmSocketHandlerTest();
        void SetUp();
        void TearDown();
    };
    
    class CAmSamplePluginStressTest: public CAmSamplePlugin
    {
        std::vector<CAmTimerStressTest2*> mTimers;
    public:
        CAmSamplePluginStressTest(CAmSocketHandler *mySocketHandler, sockType_e socketType);
        ~CAmSamplePluginStressTest();
        
        virtual void receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
        virtual bool dispatchData(const sh_pollHandle_t handle, void* userData);
        virtual bool check(const sh_pollHandle_t handle, void* userData);
        
        std::vector<CAmTimerStressTest2*> & getTimers() { return  mTimers; }
    };

} /* namespace am */
#endif /* SOCKETHANDLERTEST_H_ */
