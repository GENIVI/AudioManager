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

#include "CAmSocketHandlerTest.h"
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"


#undef ENABLED_SOCKETHANDLER_TEST_OUTPUT
#undef ENABLED_TIMERS_TEST_OUTPUT

#define SOCK_PATH "/tmp/mysock"

#define SOCKET_TEST_LOOPS_COUNT 50
#define TIMERS_TO_TEST 100

using namespace testing;
using namespace am;

static const char * TEST_SOCKET_DATA = "Got It?";
static const char * TEST_SOCKET_DATA_FINAL = "finish!";

static const std::chrono::time_point<std::chrono::high_resolution_clock> TP_ZERO;

struct TestUserData
{
    int i;
    float f;
};

struct TestStressUserData
{
    CAmSocketHandler &socket;
    std::vector<sh_pollHandle_t> &handles;
};

MockIAmSignalHandler *pMockSignalHandler = NULL;
static void signalHandler(int sig, siginfo_t *siginfo, void *context)
{
    (void) sig;
    (void) siginfo;
    (void) context;

    if(pMockSignalHandler!=NULL)
      pMockSignalHandler->signalHandler(sig, siginfo, context);

#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        std::cout<<"signal handler was called with signal " << sig << std::endl;
#endif
}

CAmSocketHandlerTest::CAmSocketHandlerTest()
{
}

CAmSocketHandlerTest::~CAmSocketHandlerTest()
{
}

void CAmSocketHandlerTest::SetUp()
{

}

void CAmSocketHandlerTest::TearDown()
{
}

CAmTimerSockethandlerController::CAmTimerSockethandlerController(CAmSocketHandler *myHandler, const timespec &timeout) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerSockethandlerController::timerCallback)
{
}

am::CAmTimerSockethandlerController::~CAmTimerSockethandlerController()
{
}

void am::CAmTimerSockethandlerController::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    mpSocketHandler->stop_listening();
}

CAmTimerSignalHandler::CAmTimerSignalHandler(CAmSocketHandler *myHandler, const timespec &timeout, const std::set<unsigned> & signals) :
        MockIAmTimerCb(), mIndex(0), mSignals(signals), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerSignalHandler::timerCallback)
{

}

am::CAmTimerSignalHandler::~CAmTimerSignalHandler()
{
}

void am::CAmTimerSignalHandler::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    if(mIndex<mSignals.size())
    {
        std::set<unsigned>::iterator it = mSignals.begin();
        std::advance(it, mIndex);
        kill(getpid(), *it);
        mIndex++;

#ifndef WITH_TIMERFD
         mpSocketHandler->updateTimer( handle, mUpdateTimeout);
#endif
    }
    else
        mpSocketHandler->stop_listening();

}

CAmTimer::CAmTimer(CAmSocketHandler *myHandler, const timespec &timeout, const int32_t repeats) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimer::timerCallback), mRepeats(repeats)
{
}

am::CAmTimer::~CAmTimer()
{
}

void am::CAmTimer::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    if (--mRepeats > 0)
    {
#ifndef WITH_TIMERFD
        mpSocketHandler->updateTimer( handle, mUpdateTimeout);
#endif
    }
    else
    {
        mpSocketHandler->stopTimer(handle);
    }
}

CAmTimerStressTest::CAmTimerStressTest(CAmSocketHandler *myHandler, const timespec &timeout, const int32_t repeats) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerStressTest::timerCallback), mRepeats(repeats), mHandle(0)
{
}

am::CAmTimerStressTest::~CAmTimerStressTest()
{
}

void am::CAmTimerStressTest::timerCallback(sh_timerHandle_t handle, void* pUserData)
{
    mpSocketHandler->removeTimer(handle);
    MockIAmTimerCb::timerCallback(handle, pUserData);
    sh_timerHandle_t handle1;
    mpSocketHandler->addTimer(mUpdateTimeout, &pTimerCallback, handle1, &(*((TestUserData*)pUserData)), true);
}

CAmTimerStressTest2::CAmTimerStressTest2(CAmSocketHandler *myHandler, const timespec &timeout, const int32_t repeats) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerStressTest2::timerCallback), mRepeats(repeats), mHandle(0)
{
}

am::CAmTimerStressTest2::~CAmTimerStressTest2()
{
}

void am::CAmTimerStressTest2::timerCallback(sh_timerHandle_t handle, void* pUserData)
{
    #ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout<<"timerCallback handle=" << handle <<std::endl;
    #endif
    MockIAmTimerCb::timerCallback(handle, pUserData);
}


CAmTimerMeasurment::CAmTimerMeasurment(CAmSocketHandler *myHandler, const timespec &timeout, const std::string & label, const int32_t repeats, void * userData) :
        MockIAmTimerCb()
        , pTimerCallback(this, &CAmTimerMeasurment::timerCallback)
        , mSocketHandler(myHandler)
        , mUpdateTimeout(timeout)
        , mUpdateTimePoint(std::chrono::seconds{ mUpdateTimeout.tv_sec } + std::chrono::nanoseconds{ mUpdateTimeout.tv_nsec })
        , mLastInvocationTime(std::chrono::high_resolution_clock::now())
        , mExpected(mUpdateTimePoint - TP_ZERO)
        , mRepeats(repeats)
        , mpUserData(userData)
        , mDebugText(label)
{
}

am::CAmTimerMeasurment::~CAmTimerMeasurment()
{
}

void am::CAmTimerMeasurment::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);

    auto t_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationLast = t_end - mLastInvocationTime;
    double diff = (std::chrono::duration<double, std::milli>(mExpected - durationLast).count());

#ifdef ENABLED_TIMERS_TEST_OUTPUT
    std::cout << mDebugText <<
    " [ expected:" <<std::chrono::duration<double, std::milli>(mExpected).count() << "ms"
    " , current:" << std::chrono::duration<double, std::milli>(durationLast).count() << "ms"
    ", diff:" << diff << "ms ] " <<
    std::endl;
#endif
    if (diff > TIMERS_CB_TOLERANCE)
        std::cout << mDebugText << " Warning [ expected:" << std::chrono::duration<double, std::milli>(mExpected).count() << "ms, current:" << std::chrono::duration<double, std::milli>(durationLast).count() << "ms ]" << std::endl;
    if (diff < -TIMERS_CB_TOLERANCE)
        std::cout << mDebugText << " Warning [ expected:" << std::chrono::duration<double, std::milli>(mExpected).count() << "ms, current:" << std::chrono::duration<double, std::milli>(durationLast).count() << "ms ]" << std::endl;

    mLastInvocationTime = t_end;
    if (--mRepeats > 0)
    {
#ifndef WITH_TIMERFD
        mSocketHandler->updateTimer( handle, mUpdateTimeout);
#endif
    }
    else
    {
        mSocketHandler->stopTimer(handle);
    }
}

void* sendTestData(int sock, const struct sockaddr* servAddr, unsigned int size, __useconds_t time = 0u)
{
    int ret =  connect(sock, servAddr, size);
    if (ret < 0)
    {
        std::cerr << "ERROR: connect() failed\n" << std::endl;
        return (NULL);
    }

    for (int i = 1; i < SOCKET_TEST_LOOPS_COUNT; i++)
    {
        std::string string(TEST_SOCKET_DATA);
        send(sock, string.c_str(), string.size(), 0);
        if (time)
            usleep(time);
    }
    std::string string(TEST_SOCKET_DATA_FINAL);
    send(sock, string.c_str(), string.size(), 0);

    return NULL;
}

void* playWithSocketServer(void* data)
{
    int sock = *((int*)data);
    struct sockaddr_in servAddr;
    unsigned short servPort = 6060;
    struct hostent *host;

    if ((host = (struct hostent*) gethostbyname("localhost")) == 0)
    {
        std::cout << "ERROR: gethostbyname() failed\n" << std::endl;
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) (host->h_addr_list[0])));
    servAddr.sin_port = htons(servPort);
    sleep(1);

    return sendTestData(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
}

void* playWithUnixSocketServer(void* data)
{
    int sock = *((int*)data);
    struct sockaddr_un servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    strcpy(servAddr.sun_path, SOCK_PATH);
    servAddr.sun_family = AF_UNIX;
    sleep(1);

    return sendTestData(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
}

void* threadCallbackUnixSocketAndTimers(void* data)
{
    int sock = *((int*)data);
    struct sockaddr_un servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    strcpy(servAddr.sun_path, SOCK_PATH);
    servAddr.sun_family = AF_UNIX;
    sleep(1);

    return sendTestData(sock, (struct sockaddr*)&servAddr, sizeof(servAddr), 500000);
}

void* threadRaceFd(void* pData)
{
    struct TestStressUserData data = *(struct TestStressUserData*)pData;
    usleep(50000);
    auto elem = data.handles.begin();
    std::advance(elem, data.handles.size() / 2);
    data.socket.removeFDPoll(*elem, sh_rmv_e::RMV_N_CLS);
    data.handles.erase(elem);

    return NULL;
}
void* threadEnd(void* pData)
{
    struct TestStressUserData data = *(struct TestStressUserData*)pData;
    usleep(1000000);
    data.socket.exit_mainloop();

    return NULL;
}

TEST(CAmSocketHandlerTest, stressTestUnixSocketAndTimers)
{

    pthread_t serverThread;

    int socket_;

    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    CAmSamplePluginStressTest::sockType_e type = CAmSamplePlugin::UNIX;
    CAmSamplePluginStressTest myplugin(&myHandler, type);

    EXPECT_CALL(myplugin,receiveData(Field(&pollfd::revents, Eq(POLL_IN)),_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));
    EXPECT_CALL(myplugin,dispatchData(_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));
    EXPECT_CALL(myplugin,check(_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));

    for(int i=0;i<myplugin.getTimers().size();i++)
    {
        EXPECT_CALL(*myplugin.getTimers()[i],timerCallback(_,_)).Times(AnyNumber());
    }


    if ((socket_ = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "socket problem" << std::endl;
    }
    ASSERT_GT(socket_, -1);

    //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, threadCallbackUnixSocketAndTimers, &socket_);

    myHandler.start_listenting();

    pthread_join(serverThread, NULL);
    shutdown(socket_, SHUT_RDWR);
}


TEST(CAmSocketHandlerTest, fdStressTest)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());

    //Check unkonw systemd fd ids
    sh_pollHandle_t handle;
    EXPECT_EQ(myHandler.addFDPoll(100, 0, NULL, NULL, NULL, NULL, NULL, handle), E_NON_EXISTENT);

    int fd(-1);
    std::vector<sh_pollHandle_t> handles(10);
    for (auto& hndl : handles)
    {
        fd = eventfd(0, 0);
        ASSERT_EQ(myHandler.addFDPoll(fd, POLL_IN, NULL, NULL, NULL, NULL, NULL, hndl), E_OK);
    }

    // remove/add check
    ASSERT_EQ(myHandler.addFDPoll(fd, POLL_IN, NULL, NULL, NULL, NULL, NULL, handles.back()), E_ALREADY_EXISTS);
    ASSERT_EQ(myHandler.removeFDPoll(handles.back()), E_OK);
    ASSERT_EQ(myHandler.addFDPoll(fd, POLL_IN, NULL, NULL, NULL, NULL, NULL, handles.back()), E_OK);

    // create a copy to check if all handles are removed
    std::vector<sh_pollHandle_t> handlesCheckup(handles);

    while (handles.size())
    {
        pthread_t tid1, tid2;

        // this removes an element before starting the socket handler and we
        // erase the last handle
        myHandler.removeFDPoll(handles.back(), sh_rmv_e::RMV_N_CLS);
        handles.erase(handles.end()-1);

        TestStressUserData data = {myHandler, handles};
        pthread_create(&tid1, NULL, threadEnd, &data);

        // erase the handle in the middle
        pthread_create(&tid2, NULL, threadRaceFd, &data);

        myHandler.start_listenting();

        pthread_join(tid2, NULL);
        pthread_join(tid1, NULL);
    }

    // now do the check
    for (auto& hndl : handlesCheckup)
        EXPECT_EQ(myHandler.removeFDPoll(hndl), E_UNKNOWN) << "Handle " << hndl << " not correctly removed before";
}

TEST(CAmSocketHandlerTest, timersOneshot)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    timespec timeoutTime;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 0;
    CAmTimer testCallback1(&myHandler, timeoutTime);

    struct TestUserData
    {
        int i;
        float f;
    };
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    sh_timerHandle_t handle;
    ASSERT_EQ(myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, &userData), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,&userData)).Times(Exactly(1));

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 3;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    ASSERT_EQ(myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(Exactly(1));
    myHandler.start_listenting();
}

TEST(CAmSocketHandlerTest, timersStop)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    timespec timeoutTime;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 0;
    CAmTimer testCallback1(&myHandler, timeoutTime, 4);

    struct TestUserData
    {
        int i;
        float f;
    };
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    sh_timerHandle_t handle;
    ASSERT_EQ(myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, &userData, true), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,&userData)).Times(4);

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 6;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    ASSERT_EQ(myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);
    myHandler.start_listenting();
}

TEST(CAmSocketHandlerTest, timersGeneral)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());

    timespec timeoutTime;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 0;
    CAmTimer testCallback1(&myHandler, timeoutTime, 4);

    struct TestUserData
    {
        int i;
        float f;
    };
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    sh_timerHandle_t handle;
    ASSERT_EQ(myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, &userData, true), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,&userData)).Times(4); //+1 because of measurment

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 5;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    ASSERT_EQ(myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);
    myHandler.start_listenting();
}

TEST(CAmSocketHandlerTest, timersStressTest)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());

    sh_timerHandle_t handle;
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 60;

    timespec timeoutTime;
    timeoutTime.tv_sec = 0;
    timeoutTime.tv_nsec = 10000000;// 0,01

    std::vector<CAmTimerStressTest*> timers(TIMERS_TO_TEST, NULL);

    for (auto & timer: timers)
    {
        timer = new CAmTimerStressTest(&myHandler, timeoutTime, 0);
        ASSERT_EQ(myHandler.addTimer(timeoutTime, &(timer->pTimerCallback), handle, &userData, true), E_OK);
        EXPECT_CALL(*timer, timerCallback(_,&userData)).Times(AnyNumber());
    }

    timespec timeoutTime11, timeout12, timeout13;
    timeoutTime11.tv_sec = 1;
    timeoutTime11.tv_nsec = 34000000;
    CAmTimerMeasurment testCallback11(&myHandler, timeoutTime11, "repeated 1", std::numeric_limits<int32_t>::max());

    timeout12.tv_sec = 0;
    timeout12.tv_nsec = 100000000;
    CAmTimerMeasurment testCallback12(&myHandler, timeout12, "repeated 2", std::numeric_limits<int32_t>::max());

    timeout13.tv_sec = 3;
    timeout13.tv_nsec = 333000000;
    CAmTimerMeasurment testCallback13(&myHandler, timeout13, "oneshot 3");

    ASSERT_EQ(myHandler.addTimer(timeoutTime11, &testCallback11.pTimerCallback, handle, NULL, true), E_OK);
    EXPECT_CALL(testCallback11,timerCallback(_,NULL)).Times(AnyNumber());

    ASSERT_EQ(myHandler.addTimer(timeout12, &testCallback12.pTimerCallback, handle, NULL, true), E_OK);
    EXPECT_CALL(testCallback12,timerCallback(_,NULL)).Times(AnyNumber());

    ASSERT_EQ(myHandler.addTimer(timeout13, &testCallback13.pTimerCallback, handle, NULL), E_OK);
    EXPECT_CALL(testCallback13,timerCallback(_,NULL)).Times(Exactly(1));


    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    ASSERT_EQ(myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL), E_OK);

    EXPECT_CALL(testCallback4,timerCallback(_,NULL)).Times(Exactly(1));
    myHandler.start_listenting();

    for (auto & timer: timers)
    {
        delete timer, timer = NULL;
    }
}


TEST(CAmSocketHandlerTest,playWithTimers)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    timespec timeoutTime, timeout2, timeout3, timeout4;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 34000000;
    CAmTimerMeasurment testCallback1(&myHandler, timeoutTime, "repeated 1", std::numeric_limits<int32_t>::max());

    timeout2.tv_nsec = 2000000;
    timeout2.tv_sec = 0;
    CAmTimerMeasurment testCallback2(&myHandler, timeout2, "repeated 2", std::numeric_limits<int32_t>::max());

    timeout3.tv_nsec = 333000000;
    timeout3.tv_sec = 3;
    CAmTimerMeasurment testCallback3(&myHandler, timeout3, "oneshot 3");

    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 8;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    sh_timerHandle_t handle;
    ASSERT_EQ(myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, NULL, true), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,NULL)).Times(AnyNumber());

    ASSERT_EQ(myHandler.addTimer(timeout2, &testCallback2.pTimerCallback, handle, NULL, true), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback2,timerCallback(handle,NULL)).Times(AnyNumber());

    ASSERT_EQ(myHandler.addTimer(timeout3, &testCallback3.pTimerCallback, handle, NULL), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 3);
#else
    ASSERT_EQ(handle, 4);
#endif
    EXPECT_CALL(testCallback3,timerCallback(handle,NULL)).Times(Exactly(1));

    ASSERT_EQ(myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL), E_OK);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 4);
#else
    ASSERT_EQ(handle, 5);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);

    myHandler.start_listenting();
}



TEST(CAmSocketHandlerTest, signalHandlerPrimaryPlusSecondary)
{
    pMockSignalHandler = new MockIAmSignalHandler;
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    ASSERT_EQ(myHandler.listenToSignals({SIGHUP}), E_OK);
    ASSERT_EQ(myHandler.listenToSignals({SIGHUP, SIGTERM, SIGCHLD}), E_OK);

    sh_pollHandle_t signalHandler1, signalHandler2;
    std::string userData = "User data";

//     critical signals are registered here:
    struct sigaction signalAction;
    memset(&signalAction, '\0', sizeof(signalAction));
    signalAction.sa_sigaction = &signalHandler;
    signalAction.sa_flags = SA_RESETHAND | SA_NODEFER| SA_SIGINFO;
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGQUIT, &signalAction, NULL);

    myHandler.addSignalHandler([&](const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)
    {
        unsigned sig = info.ssi_signo;
        pMockSignalHandler->signalHandlerAction(handle, sig, userData);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        unsigned user = info.ssi_uid;
        std::cout<<"signal handler was called from user "<< user << " with signal " << sig << std::endl;
#endif
    }, signalHandler1, &userData);
    ASSERT_EQ(signalHandler1, 1);

    myHandler.addSignalHandler([&](const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)
    {
        unsigned sig = info.ssi_signo;
        pMockSignalHandler->signalHandlerAction(handle, sig, userData);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        unsigned user = info.ssi_uid;
        std::cout<<"signal handler was called from user "<< user << " with signal " << sig << std::endl;
#endif
    }, signalHandler2, &userData);
    ASSERT_EQ(signalHandler2, 2);

    timespec timeout4;
    timeout4.tv_nsec = 200000000;
    timeout4.tv_sec = 0;
    std::set<unsigned> secondarySignals;
    secondarySignals.insert({SIGHUP,SIGTERM, SIGCHLD});
    std::set<unsigned> primarySignals({SIGQUIT,SIGINT});
    std::set<unsigned> signals(primarySignals);
    signals.insert(secondarySignals.begin(), secondarySignals.end());

    CAmTimerSignalHandler testCallback4(&myHandler, timeout4, signals);
    sh_timerHandle_t handle;

    ASSERT_EQ(myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL, true), E_OK);
#ifndef WITH_TIMERFD
   ASSERT_EQ(handle, 1);
#else
   ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(signals.size()+1);
   for(auto it: secondarySignals)
       EXPECT_CALL(*pMockSignalHandler,signalHandlerAction(signalHandler1,it,&userData)).Times(1);
   for(auto it: secondarySignals)
       EXPECT_CALL(*pMockSignalHandler,signalHandlerAction(signalHandler2,it,&userData)).Times(1);
   for(auto it: primarySignals)
       EXPECT_CALL(*pMockSignalHandler,signalHandler(it,_,_)).Times(1);

    myHandler.start_listenting();
   delete pMockSignalHandler;
}

TEST(CAmSocketHandlerTest,playWithUNIXSockets)
{
    pthread_t serverThread;
    struct sockaddr_un servAddr;
    int socket_;

    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    CAmSamplePlugin::sockType_e type = CAmSamplePlugin::UNIX;
    CAmSamplePlugin myplugin(&myHandler, type);
    ASSERT_TRUE(myplugin.isSocketOpened());

    EXPECT_CALL(myplugin,receiveData(Field(&pollfd::revents, Eq(POLL_IN)),_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));
    EXPECT_CALL(myplugin,dispatchData(_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));
    EXPECT_CALL(myplugin,check(_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));

    if ((socket_ = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "socket problem" << std::endl;
    }
    ASSERT_GT(socket_, -1);
    //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, playWithUnixSocketServer, &socket_);

    myHandler.start_listenting();

    pthread_join(serverThread, NULL);
    shutdown(socket_, SHUT_RDWR);
}

TEST(CAmSocketHandlerTest,playWithSockets)
{
    pthread_t serverThread;
    int socket_;

    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    CAmSamplePlugin::sockType_e type = CAmSamplePlugin::INET;
    CAmSamplePlugin myplugin(&myHandler, type);
    ASSERT_TRUE(myplugin.isSocketOpened());
    EXPECT_CALL(myplugin,receiveData(Field(&pollfd::revents, Eq(POLL_IN)),_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));
    EXPECT_CALL(myplugin,dispatchData(_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));
    EXPECT_CALL(myplugin,check(_,_)).Times(Exactly(SOCKET_TEST_LOOPS_COUNT));

    if ((socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        std::cout << "socket problem" << std::endl;
    }
    ASSERT_GT(socket_, -1);
        //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, playWithSocketServer, &socket_);

    myHandler.start_listenting();

    pthread_join(serverThread, NULL);
    shutdown(socket_, SHUT_RDWR);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

am::CAmSamplePlugin::CAmSamplePlugin(CAmSocketHandler *mySocketHandler, sockType_e socketType) :
        MockSocketHandlerCb(), connectFiredCB(this, &CAmSamplePlugin::connectSocket), //
        receiveFiredCB(this, &CAmSamplePlugin::receiveData), //
        sampleDispatchCB(this, &CAmSamplePlugin::dispatchData), //
        sampleCheckCB(this, &CAmSamplePlugin::check), //
        mSocketHandler(mySocketHandler), //
        mConnecthandle(), //
        mReceiveHandle(), //
        msgList(),
        mSocket(-1)
{
    int yes = 1;

    struct sockaddr_in servAddr;
    struct sockaddr_un unixAddr;
    unsigned int servPort = 6060;

    switch (socketType)
    {
        case UNIX:
            mSocket = socket(AF_UNIX, SOCK_STREAM, 0);
            if(mSocket==-1)
                return;
            unixAddr.sun_family = AF_UNIX;
            strcpy(unixAddr.sun_path, SOCK_PATH);
            unlink(unixAddr.sun_path);
            bind(mSocket, (struct sockaddr *) &unixAddr, strlen(unixAddr.sun_path) + sizeof(unixAddr.sun_family));
            break;
        case INET:
            mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(mSocket==-1)
                return;
            setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
            memset(&servAddr, 0, sizeof(servAddr));
            servAddr.sin_family = AF_INET;
            servAddr.sin_addr.s_addr = INADDR_ANY;
            servAddr.sin_port = htons(servPort);
            bind(mSocket, (struct sockaddr *) &servAddr, sizeof(servAddr));
            break;
        default:
            break;
    }

    if (listen(mSocket, 3) < 0)
    {
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        std::cout << "listen ok" << std::endl;
#endif
    } /* if */

    int a = 1;
    ioctl(mSocket, FIONBIO, (char *) &a);
    setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, (char *) &a, sizeof(a));

    short events = 0;
    events |= POLLIN;
    mySocketHandler->addFDPoll(mSocket, events, NULL, &connectFiredCB, NULL, NULL, NULL, mConnecthandle);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "setup server - listening" << std::endl;
#endif
}

void am::CAmSamplePlugin::connectSocket(const pollfd pollfd1, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //first, accept the connection, create a new filedescriptor
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "Got a connection request !" << std::endl;
#endif
    struct sockaddr answer;
    socklen_t len = sizeof(answer);
    int receiveFD = accept(pollfd1.fd, (struct sockaddr*) &answer, &len);

    //set the correct event:
    short event = 0;
    event |= POLLIN;

    //aded the filedescriptor to the sockethandler and register the callbacks for receiving the data
    mSocketHandler->addFDPoll(receiveFD, event, NULL, &receiveFiredCB, &sampleCheckCB, &sampleDispatchCB, NULL, mReceiveHandle);

}

void am::CAmSamplePlugin::receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    MockSocketHandlerCb::receiveData(pollfd, handle, userData);
    //initialize buffer
    char buffer[10];
    //read until buffer is full or no more data is there
    int read = recv(pollfd.fd, buffer, 7, 0);
    if (read > 1)
    {
        //read the message and store it in a queue
        std::string msg = std::string(buffer, read);
        msgList.push(msg);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        std::cout << "Got a message !" << std::endl;
#endif
    }
}

bool am::CAmSamplePlugin::dispatchData(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //read data from the queue
    MockSocketHandlerCb::dispatchData(handle, userData);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "Data:" << msgList.front() << std::endl;
#endif
    //if the message was our finish message, we quit the poll loop
    if (!(msgList.front().compare(TEST_SOCKET_DATA_FINAL) == 0 || msgList.front().compare(TEST_SOCKET_DATA) == 0)) //error
    {
        mSocketHandler->stop_listening();
    }
    if (msgList.front().compare(TEST_SOCKET_DATA_FINAL) == 0) //ok
    {
        mSocketHandler->stop_listening();
    }

    //remove the message from the queue and return false if there is no more message to read.
    msgList.pop();
    if (msgList.size() != 0)
        return true;
    return false;
}

bool am::CAmSamplePlugin::check(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    MockSocketHandlerCb::check(handle, userData);
    //checks if there is data to dispatch
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "check!:" << std::endl;
#endif
    if (msgList.size() != 0)
        return true;
    return false;
}

CAmSamplePluginStressTest::CAmSamplePluginStressTest(CAmSocketHandler *mySocketHandler, sockType_e socketType):CAmSamplePlugin(mySocketHandler,socketType)
, mTimers(TIMERS_TO_TEST, NULL)
{
    sh_timerHandle_t handle;
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;
    timespec timeoutTime;
    timeoutTime.tv_sec = 0;
    timeoutTime.tv_nsec = 10000000;// 0,01
    for (auto & timer : mTimers)
    {
        timer = new CAmTimerStressTest2(mySocketHandler, timeoutTime, 0);
        if (mySocketHandler->addTimer(timeoutTime, &(timer->pTimerCallback), handle, &userData, true) == E_OK);
            timer->setHandle(handle);

        EXPECT_CALL(*timer, timerCallback(_,&userData)).Times(AnyNumber());
    }
}

CAmSamplePluginStressTest::~CAmSamplePluginStressTest()
{
    for (auto & timer : mTimers)
    {
        delete timer, timer = NULL;
    }
}

void CAmSamplePluginStressTest::receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
{
    CAmSamplePlugin::receiveData(pollfd, handle, userData);

    sh_timerHandle_t handle1;
    for (auto & timer : mTimers)
    {
        ASSERT_EQ(mSocketHandler->removeTimer(timer->getHandle()), E_OK);
        ASSERT_EQ(mSocketHandler->addTimer(timer->getUpdateTimeout(), &(timer->pTimerCallback), handle1, NULL, true), E_OK);
        timer->setHandle(handle1);
    }
}

bool CAmSamplePluginStressTest::dispatchData(const sh_pollHandle_t handle, void* userData)
{
    return CAmSamplePlugin::dispatchData( handle, userData);
}

bool CAmSamplePluginStressTest::check(const sh_pollHandle_t handle, void* userData)
{
    return CAmSamplePlugin::check( handle, userData);
}
